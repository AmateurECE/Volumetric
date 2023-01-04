///////////////////////////////////////////////////////////////////////////////
// NAME:            docker.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic implementing Docker proxy object.
//
// CREATED:         01/18/2022
//
// LAST EDITED:     01/03/2023
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

#include <volumetric/docker.h>
#include <volumetric/docker/internal.h>

typedef struct DockerVolumeListIter {
    GPtrArray* array;
    guint index;
} DockerVolumeListIter;

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static void populate_docker_volume_from_json(DockerVolume* volume,
                                             json_object* object) {
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

///////////////////////////////////////////////////////////////////////////////
// Public API
////

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
    int result =
        http_post_application_json(docker, "http://localhost/volumes/create");
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
    json_object* volumes =
        json_object_object_get(docker->write_object, "Volumes");
    if (NULL == volumes) {
        json_object* message =
            json_object_object_get(docker->write_object, "message");
        fprintf(stderr, "%s:%d:Docker daemon says: %s\n", __FILE__, __LINE__,
                json_object_get_string(message));
        free(iter);
        json_object_put(docker->write_object);
        return NULL;
    }

    iter->array =
        g_ptr_array_new_with_free_func((GDestroyNotify)docker_volume_free);
    int array_length = json_object_array_length(volumes);
    for (int i = 0; i < array_length; ++i) {
        json_object* object = json_object_array_get_idx(volumes, i);
        DockerVolume* volume = malloc(sizeof(DockerVolume));
        assert(NULL != volume);
        populate_docker_volume_from_json(volume, object);
        g_ptr_array_add(iter->array, volume);
    }

    json_object_put(docker->write_object);
    return iter;
}

const DockerVolume* docker_volume_list_iter_next(DockerVolumeListIter* iter) {
    if (iter->index >= iter->array->len) {
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
    json_object* message =
        json_object_object_get(docker->write_object, "message");
    if (NULL != message) {
        fprintf(stderr, "%s:%d:Docker daemon says: %s\n", __FILE__, __LINE__,
                json_object_get_string(message));
        return NULL;
    }

    DockerVolume* volume = malloc(sizeof(DockerVolume));
    assert(NULL != volume);
    populate_docker_volume_from_json(volume, docker->write_object);
    json_object_put(docker->write_object);
    return volume;
}

void docker_volume_free(DockerVolume* volume) {
    if (NULL != volume->name) {
        free(volume->name);
    }
    if (NULL != volume->driver) {
        free(volume->driver);
    }
    if (NULL != volume->mountpoint) {
        free(volume->mountpoint);
    }
    free(volume);
}

int docker_volume_remove(Docker* proxy, const char* name) {
    static const char* url_base = "http://localhost/volumes/";
    size_t url_length = strlen(url_base) + strlen(name);
    char* request_url = malloc(url_length + 1);
    assert(NULL != request_url);

    memset(request_url, 0, url_length + 1);
    strcat(request_url, url_base);
    strcat(request_url, name);

    int result = http_delete(proxy, request_url);
    free(request_url);
    return result;
}

int docker_volume_exists(Docker* docker, const char* name) {
    DockerVolumeListIter* iter = docker_volume_list(docker);
    if (NULL == iter) {
        // We couldn't connect to the server for some reason.
        return -ENOTCONN;
    }

    const DockerVolume* list_entry = NULL;
    bool exists = false;
    while (NULL != (list_entry = docker_volume_list_iter_next(iter))) {
        if (!strcmp(name, list_entry->name)) {
            exists = true;
            break;
        }
    }

    docker_volume_list_iter_free(iter);

    if (exists) {
        return 1;
    } else {
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
