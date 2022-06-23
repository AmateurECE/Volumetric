///////////////////////////////////////////////////////////////////////////////
// NAME:            container.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for working with Docker containers.
//
// CREATED:         02/13/2022
//
// LAST EDITED:     06/22/2022
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
#include <stdlib.h>
#include <stdio.h>

#include <glib-2.0/glib.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>

#include <volumetric/docker.h>
#include <volumetric/docker/internal.h>
#include <volumetric/string-handling.h>

typedef struct DockerContainerIter {
    GPtrArray* array;
    guint index;
} DockerContainerIter;

typedef struct DockerMountIter {
    GPtrArray* array;
    guint index;
} DockerMountIter;

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static void populate_docker_mount_from_json(DockerMount* mount,
    json_object* object)
{
    struct json_object_iterator iter = json_object_iter_begin(object);
    struct json_object_iterator end = json_object_iter_end(object);

    while (!json_object_iter_equal(&iter, &end)) {
        const char* key = json_object_iter_peek_name(&iter);
        json_object* value = json_object_iter_peek_value(&iter);

        if (!strcmp("Source", key)) {
            mount->source = strdup(json_object_get_string(value));
        }

        json_object_iter_next(&iter);
    }
}

static DockerMountIter* docker_mount_iter_new(json_object* array) {
    DockerMountIter* iter = malloc(sizeof(DockerMountIter));
    assert(NULL != iter);
    memset(iter, 0, sizeof(*iter));

    iter->array = g_ptr_array_new_with_free_func(
        (GDestroyNotify)docker_mount_free);
    int array_length = json_object_array_length(array);
    for (int i = 0; i < array_length; ++i) {
        json_object* object = json_object_array_get_idx(array, i);
        DockerMount* mount = malloc(sizeof(DockerMount));
        assert(NULL != mount);
        populate_docker_mount_from_json(mount, object);
        g_ptr_array_add(iter->array, mount);
    }
    return iter;
}

static void populate_docker_container_from_json(DockerContainer* container,
    json_object* object)
{
    struct json_object_iterator iter = json_object_iter_begin(object);
    struct json_object_iterator end = json_object_iter_end(object);

    while (!json_object_iter_equal(&iter, &end)) {
        const char* key = json_object_iter_peek_name(&iter);
        json_object* value = json_object_iter_peek_value(&iter);

        if (!strcmp("Id", key)) {
            container->id = strdup(json_object_get_string(value));
        } else if (!strcmp("Mounts", key) && NULL != value) {
            container->mounts = docker_mount_iter_new(value);
        }

        json_object_iter_next(&iter);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

// List the containers the engine is managing
DockerContainerIter* docker_container_list(Docker* docker)
{
    DockerContainerIter* iter = malloc(sizeof(DockerContainerIter));
    if (NULL == iter) {
        fprintf(stderr, "%s:%d:%s\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }
    memset(iter, 0, sizeof(*iter));

    int result = http_get_application_json(docker,
        "http://localhost/containers/json");
    if (0 != result) {
        free(iter);
        return NULL;
    }

    // Check whether the response was an error message
    if (!json_object_is_type(docker->write_object, json_type_array)) {
        json_object* message = json_object_object_get(docker->write_object,
            "message");
        fprintf(stderr, "%s:%d:Docker daemon says: %s\n", __FILE__, __LINE__,
            json_object_get_string(message));
        free(iter);
        json_object_put(docker->write_object);
        return NULL;
    }

    iter->array = g_ptr_array_new_with_free_func(
        (GDestroyNotify)docker_container_free);
    int array_length = json_object_array_length(docker->write_object);
    for (int i = 0; i < array_length; ++i) {
        json_object* object = json_object_array_get_idx(docker->write_object,
            i);
        DockerContainer* container = malloc(sizeof(DockerContainer));
        assert(NULL != container);
        populate_docker_container_from_json(container, object);
        g_ptr_array_add(iter->array, container);
    }

    json_object_put(docker->write_object);
    return iter;
}

// Mutate the iterator
const DockerContainer* docker_container_iter_next(DockerContainerIter* iter) {
    if (iter->index >= iter->array->len) {
        return NULL;
    }

    return iter->array->pdata[iter->index++];
}

void docker_container_iter_free(DockerContainerIter* iter) {
    g_ptr_array_unref(iter->array);
    free(iter);
}

// Iterate through mounts
const DockerMount* docker_mount_iter_next(DockerMountIter* iter) {
    if (iter->index >= iter->array->len) {
        return NULL;
    }

    return iter->array->pdata[iter->index++];
}

void docker_mount_iter_free(DockerMountIter* iter) {
    g_ptr_array_unref(iter->array);
    free(iter);
}

void docker_container_free(DockerContainer* container) {
    if (NULL != container->id) { free(container->id); }
    if (NULL != container->mounts) {
        docker_mount_iter_free(container->mounts);
    }
    free(container);
}

void docker_mount_free(DockerMount* mount) {
    if (NULL != mount->source) { free(mount->source); }
    free(mount);
}

int docker_container_pause(Docker* docker, const char* container_id)
{
    char* pause_url = string_append_new(
        string_append_new(strdup("http://localhost/containers/"),
            container_id),
        "/pause");
    assert(NULL != pause_url);
    int result = http_post(docker, pause_url);
    free(pause_url);

    if (NULL != docker->write_object) {
        json_object* message = json_object_object_get(docker->write_object,
            "message");
        fprintf(stderr, "%s:%d:Docker daemon says: %s\n", __FUNCTION__,
            __LINE__, json_object_get_string(message));
        json_object_put(docker->write_object);
        return 1;
    }

    return result;
}

int docker_container_unpause(Docker* docker, const char* container_id)
{
    char* unpause_url = string_append_new(
        string_append_new(strdup("http://localhost/containers/"),
            container_id),
        "/unpause");
    assert(NULL != unpause_url);
    int result = http_post(docker, unpause_url);
    free(unpause_url);

    if (NULL != docker->write_object) {
        json_object* message = json_object_object_get(docker->write_object,
            "message");
        fprintf(stderr, "%s:%d:Docker daemon says: %s\n", __FUNCTION__,
            __LINE__, json_object_get_string(message));
        json_object_put(docker->write_object);
        return 1;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
