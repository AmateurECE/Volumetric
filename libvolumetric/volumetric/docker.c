///////////////////////////////////////////////////////////////////////////////
// NAME:            docker.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic implementing Docker proxy object.
//
// CREATED:         01/18/2022
//
// LAST EDITED:     02/13/2022
//
// Copyright 2022, Ethan D. Twardy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <glib-2.0/glib.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>
#include <json-c/json_visit.h>

#include <volumetric/docker.h>

typedef struct DockerVolumeListIter {
    GPtrArray* array;
    guint index;
} DockerVolumeListIter;

///////////////////////////////////////////////////////////////////////////////
// Private API
////

typedef struct Docker {
    CURL* curl;

    json_tokener* tokener;
    json_object* write_object;
    char* read_object;
    size_t read_object_length;
    size_t read_object_index;

    union {
        // Data for the list call.
        struct {
            int (*visitor)(const DockerVolume*, void*);
            void* visitor_data;
        } list;
    };
} Docker;

static const char* DOCKER_SOCK_PATH = "/var/run/docker.sock";

static void docker_volume_free_impl(DockerVolume* volume, bool free_container)
{
    if (NULL != volume->name) { free(volume->name); }
    if (NULL != volume->driver) { free(volume->driver); }
    if (NULL != volume->mountpoint) { free(volume->mountpoint); }
    if (free_container) { free(volume); }
}

static int http_encode(json_object* object, char** string, size_t* length) {
    const char* string_value = json_object_to_json_string_ext(object,
        JSON_C_TO_STRING_NOSLASHESCAPE);
    *length = strlen(string_value) + 2; // For "\r\n"
    *string = malloc(*length + 1);
    if (NULL == *string) {
        fprintf(stderr, "%s:%d: Couldn't allocate memory: %s\n", __FILE__,
            __LINE__, strerror(errno));
        return -ENOMEM;
    }

    memset(*string, 0, *length + 1);
    strcat(*string, string_value);
    strcat(*string, "\r\n");
    (*string)[*length] = '\0';
    return 0;
}

static size_t copy_data_to_curl_request(char* buffer,
    size_t size __attribute__((unused)), size_t nitems, void* user_data)
{
    Docker* docker = (Docker*)user_data;
    size_t copy_length = docker->read_object_length
        - docker->read_object_index;
    if (nitems < copy_length) {
        copy_length = nitems;
    }

    memcpy(buffer, docker->read_object + docker->read_object_index,
        copy_length);
    docker->read_object_index += copy_length;
    return copy_length;
}

static size_t copy_data_from_curl_response(void* buffer,
    size_t size __attribute__((unused)), size_t nmemb, void* user_data)
{
    Docker* docker = (Docker*)user_data;
    docker->write_object = json_tokener_parse_ex(docker->tokener, buffer,
        nmemb);
    enum json_tokener_error error = json_tokener_get_error(docker->tokener);
    if (NULL == docker->write_object && json_tokener_continue != error) {
        return 0;
    }

    return nmemb;
}

static void populate_docker_volume_from_json(DockerVolume* volume,
    json_object* object)
{
    struct json_object_iterator iter = json_object_iter_begin(object);
    struct json_object_iterator end = json_object_iter_end(object);

    while (!json_object_iter_equal(&iter, &end)) {
        const char* key = json_object_iter_peek_name(&iter);
        json_object* value = json_object_iter_peek_value(&iter);

        if (!strcmp("Name", key)) {
            volume->name = strdup(json_object_get_string(value));
        } else if (!strcmp("Driver", key)) {
            volume->driver = strdup(json_object_get_string(value));
        } else if (!strcmp("Mountpoint", key)) {
            volume->mountpoint = strdup(json_object_get_string(value));
        }

        json_object_iter_next(&iter);
    }
}

static int http_get_application_json(Docker* docker, const char* url) {
    docker->tokener = json_tokener_new();
    curl_easy_setopt(docker->curl, CURLOPT_URL, url);
    curl_easy_setopt(docker->curl, CURLOPT_WRITEFUNCTION,
        copy_data_from_curl_response);
    curl_easy_setopt(docker->curl, CURLOPT_WRITEDATA, docker);

    char error_buffer[CURL_ERROR_SIZE] = {0};
    curl_easy_setopt(docker->curl, CURLOPT_ERRORBUFFER, error_buffer);

    CURLcode response = CURLE_OK;
    response = curl_easy_perform(docker->curl);
    int result = 0;

    // Check that CURL is happy
    if (CURLE_OK != response) {
        fprintf(stderr, "%s:%d:Couldn't connect to docker daemon: %s (%s)\n",
            __FILE__, __LINE__, error_buffer, curl_easy_strerror(response));
        json_tokener_free(docker->tokener);
        return -EINVAL;
    }

    // Check that the docker daemon returned a valid JSON object
    if (NULL == docker->write_object) {
        enum json_tokener_error error = json_tokener_get_error(
            docker->tokener);
        fprintf(stderr, "%s:%d:Couldn't enumerate docker volumes: %s\n",
            __FILE__, __LINE__, json_tokener_error_desc(error));
        json_tokener_free(docker->tokener);
        return -EINVAL;
    }

    json_tokener_free(docker->tokener);
    return result;
}

static int http_post_application_json(Docker* docker, const char* url) {
    curl_easy_setopt(docker->curl, CURLOPT_POST, 1);
    curl_easy_setopt(docker->curl, CURLOPT_READFUNCTION,
        copy_data_to_curl_request);
    curl_easy_setopt(docker->curl, CURLOPT_READDATA, docker);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(docker->curl, CURLOPT_HTTPHEADER, headers);
    return http_get_application_json(docker, url);
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

Docker* docker_proxy_new() {
    Docker* docker = malloc(sizeof(Docker));
    if (NULL == docker) {
        return NULL;
    }

    memset(docker, 0, sizeof(Docker));
    docker->curl = curl_easy_init();
    curl_easy_setopt(docker->curl, CURLOPT_UNIX_SOCKET_PATH, DOCKER_SOCK_PATH);
    return docker;
}

void docker_proxy_free(Docker* docker) {
    curl_easy_cleanup(docker->curl);
    free(docker);
}

DockerVolume* docker_volume_create(Docker* docker, const char* name) {
    // Create json_object for request
    json_object* request = json_object_new_object();
    json_object_object_add(request, "Name", json_object_new_string(name));
    docker->read_object = NULL;
    if (0 != http_encode(request, &docker->read_object,
            &docker->read_object_length)) {
        goto error;
    }

    docker->read_object_index = 0;
    int result = http_post_application_json(docker,
        "http://localhost/volumes/create");
    json_object_put(request);
    free(docker->read_object);
    if (0 != result) {
        goto error;
    }


    // Read json_object from response
    DockerVolume* volume = malloc(sizeof(DockerVolume));
    if (NULL == volume) {
        fprintf(stderr, "%s:%d: Failed to initialize memory for volume: %s\n",
            __FILE__, __LINE__, strerror(errno));
        goto error;
    }

    populate_docker_volume_from_json(volume, docker->write_object);
    return volume;
 error:
    return NULL;
}

DockerVolumeListIter* docker_volume_list(Docker* docker) {
    DockerVolumeListIter* iter = malloc(sizeof(DockerVolumeListIter));
    if (NULL == iter) {
        fprintf(stderr, "%s:%d:%s\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }
    memset(iter, 0, sizeof(*iter));

    int result = http_get_application_json(docker, "http://localhost/volumes");
    if (0 != result) {
        free(iter);
        return NULL;
    }

    // Check whether the response was an error message
    json_object* volumes = json_object_object_get(docker->write_object,
        "Volumes");
    if (NULL == volumes) {
        json_object* message = json_object_object_get(docker->write_object,
            "message");
        fprintf(stderr, "%s:%d:Docker daemon says: %s\n", __FILE__, __LINE__,
            json_object_get_string(message));
        free(iter);
        json_object_put(docker->write_object);
        return NULL;
    }

    iter->array = g_ptr_array_new_with_free_func(
        (GDestroyNotify)docker_volume_free);
    int array_length = json_object_array_length(volumes);
    for (int i = 0; i < array_length; ++i) {
        json_object* object = json_object_array_get_idx(volumes, i);
        DockerVolume* volume = malloc(sizeof(DockerVolume));
        assert(NULL != volume);
        populate_docker_volume_from_json(volume, object);
        g_ptr_array_add(iter->array, volume);
    }

    return iter;
}

const DockerVolume* docker_volume_list_iter_next(DockerVolumeListIter* iter) {
    if (iter->index > iter->array->len) {
        return NULL;
    }

    return iter->array->pdata[iter->index++];
}

void docker_volume_list_iter_free(DockerVolumeListIter* iter) {
    g_ptr_array_unref(iter->array);
    free(iter);
}

DockerVolume* docker_volume_inspect(Docker* docker, const char* name) {
    static const char* url_base = "http://localhost/volumes/";
    size_t url_base_length = strlen(url_base);
    size_t url_length = url_base_length + strlen(name);
    char* url = malloc(url_length + 1);
    assert(NULL != url);

    memset(url, 0, url_length + 1);
    strcat(url, url_base);
    strcat(url, name);

    int result = http_get_application_json(docker, url);
    free(url);
    if (0 != result) {
        return NULL;
    }

    // Read json_object from response
    json_object* message = json_object_object_get(docker->write_object,
        "message");
    if (NULL != message) {
        fprintf(stderr, "%s:%d:Docker daemon says: %s\n", __FILE__, __LINE__,
            json_object_get_string(message));
        return NULL;
    }

    DockerVolume* volume = malloc(sizeof(DockerVolume));
    assert(NULL != volume);
    populate_docker_volume_from_json(volume, docker->write_object);
    return volume;
}

void docker_volume_free(DockerVolume* volume)
{ docker_volume_free_impl(volume, true); }

// Currently no use case for this?
int docker_volume_remove(Docker* proxy, const char* name);

///////////////////////////////////////////////////////////////////////////////
// Container API
////

// List the containers the engine is managing
DockerContainerIter* docker_container_list(Docker* docker)
{ return NULL; }

// Mutate the iterator
const DockerContainer* docker_container_iter_next(DockerContainerIter* iter)
{ return NULL; }

void docker_container_iter_free(DockerContainerIter* iter) {}

// Iterate through mounts
const DockerMount* docker_mount_iter_next(DockerMountIter* iter)
{ return NULL; }

void docker_mount_iter_free(DockerMountIter* iter) {}

int docker_container_pause(Docker* docker, const char* container_name)
{ return 0; }

int docker_container_unpause(Docker* docker, const char* container_name)
{ return 0; }

///////////////////////////////////////////////////////////////////////////////
