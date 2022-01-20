///////////////////////////////////////////////////////////////////////////////
// NAME:            docker.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic implementing Docker proxy object.
//
// CREATED:         01/18/2022
//
// LAST EDITED:     01/19/2022
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

#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <glib-2.0/glib.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>
#include <json-c/json_visit.h>

#include <docker.h>

///////////////////////////////////////////////////////////////////////////////
// Private API
////

typedef struct Docker {
    CURL* curl;

    json_tokener* tokener;
    json_object* object;

    union {
        // Data for the list call.
        struct {
            int (*visitor)(const DockerVolume*, void*);
            void* visitor_data;
        } list;
    };
} Docker;

static const char* DOCKER_SOCK_PATH = "/var/run/docker.sock";

static size_t copy_data_from_curl_response(void* buffer,
    size_t size __attribute__((unused)), size_t nmemb, void* user_data)
{
    Docker* docker = (Docker*)user_data;
    docker->object = json_tokener_parse_ex(docker->tokener, buffer, nmemb);
    enum json_tokener_error error = json_tokener_get_error(docker->tokener);
    if (NULL == docker->object && json_tokener_continue != error) {
        return 0;
    }

    return nmemb;
}

static int invoke_visitor_for_volume(Docker* docker, json_object* object) {
    struct json_object_iterator iter = json_object_iter_begin(object);
    struct json_object_iterator end = json_object_iter_end(object);

    DockerVolume volume = {0};
    while (!json_object_iter_equal(&iter, &end)) {
        const char* key = json_object_iter_peek_name(&iter);
        json_object* value = json_object_iter_peek_value(&iter);

        if (!strcmp("Name", key)) {
            volume.name = json_object_to_json_string_ext(value,
                JSON_C_TO_STRING_NOSLASHESCAPE);
        } else if (!strcmp("Driver", key)) {
            volume.driver = json_object_to_json_string_ext(value,
                JSON_C_TO_STRING_NOSLASHESCAPE);
        } else if (!strcmp("Mountpoint", key)) {
            volume.mountpoint = json_object_to_json_string_ext(value,
                JSON_C_TO_STRING_NOSLASHESCAPE);
        }

        json_object_iter_next(&iter);
    }

    return docker->list.visitor(&volume, docker->list.visitor_data);
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
    free(docker);
}

int docker_volume_create(Docker* proxy, const char* name, const char* driver);

int docker_volume_list(Docker* docker,
    int (*visitor)(const DockerVolume*, void*), void* user_data)
{
    docker->tokener = json_tokener_new();
    docker->list.visitor = visitor;
    docker->list.visitor_data = user_data;

    curl_easy_setopt(docker->curl, CURLOPT_URL, "http://localhost/volumes");
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
        result = -EINVAL;
        goto error;
    }

    // Check that the docker daemon returned a valid JSON object
    if (NULL == docker->object) {
        enum json_tokener_error error = json_tokener_get_error(
            docker->tokener);
        fprintf(stderr, "%s:%d:Couldn't enumerate docker volumes: %s\n",
            __FILE__, __LINE__, json_tokener_error_desc(error));
        result = -EINVAL;
        goto error;
    }

    // Check whether the response was an error message
    json_object* volumes = json_object_object_get(docker->object, "Volumes");
    if (NULL == volumes) {
        json_object* message = json_object_object_get(docker->object,
            "message");
        fprintf(stderr, "%s:%d:Docker daemon says: %s\n", __FILE__, __LINE__,
            json_object_get_string(message));
        result = -EINVAL;
        goto error;
    }

    int array_length = json_object_array_length(volumes);
    for (int i = 0; i < array_length; ++i) {
        json_object* volume = json_object_array_get_idx(volumes, i);
        int callback_result = invoke_visitor_for_volume(docker, volume);
        if (DOCKER_VISITOR_STOP == callback_result) {
            break;
        }
    }

 error:
    json_tokener_free(docker->tokener);
    return result;
}

// Currently no use case for these?
int docker_volume_inspect(Docker* proxy, const char* name,
    void (*visitor)(const DockerVolume*));
int docker_volume_remove(Docker* proxy, const char* name);

///////////////////////////////////////////////////////////////////////////////
