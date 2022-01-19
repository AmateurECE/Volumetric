///////////////////////////////////////////////////////////////////////////////
// NAME:            docker.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic implementing Docker proxy object.
//
// CREATED:         01/18/2022
//
// LAST EDITED:     01/18/2022
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
    union {
        // Data for the list call.
        struct {
            json_tokener* tokener;
            json_object* object;
            int (*visitor)(DockerVolume*, void*);
            void* visitor_data;
        } list;
    };
} Docker;

static const char* DOCKER_SOCK_PATH = "/var/run/docker.sock";

static size_t priv_curl_list(void* buffer, size_t size __attribute__((unused)),
    size_t nmemb, void* user_data)
{
    Docker* docker = (Docker*)user_data;
    docker->list.object = json_tokener_parse_ex(docker->list.tokener, buffer,
        nmemb);
    enum json_tokener_error error = json_tokener_get_error(
        docker->list.tokener);
    if (NULL == docker->list.object && json_tokener_continue != error) {
        return 0;
    }

    return nmemb;
}

static int priv_visit_list(json_object* object, int flags, json_object* parent,
    const char* key, size_t* index, void* user_data)
{
    /* Docker* docker = (Docker*)user_data; */
    printf("visit: %d", flags & JSON_C_VISIT_SECOND);
    if (NULL != key) {
        printf(",key=%s", key);
    }

    if (NULL != index) {
        printf(",index=%zu", *index);
    }

    printf("\n");
    return 0;
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

int docker_volume_list(Docker* docker, int (*visitor)(DockerVolume*, void*),
    void* user_data)
{
    docker->list.tokener = json_tokener_new();
    docker->list.visitor = visitor;
    docker->list.visitor_data = user_data;

    curl_easy_setopt(docker->curl, CURLOPT_URL, "http://localhost/volumes");
    curl_easy_setopt(docker->curl, CURLOPT_WRITEFUNCTION, priv_curl_list);
    curl_easy_setopt(docker->curl, CURLOPT_WRITEDATA, docker);
    CURLcode response = curl_easy_perform(docker->curl);
    int result = 0;
    if (CURLE_OK != response) {
        fprintf(stderr, "Couldn't enumerate docker volumes: %s\n",
            curl_easy_strerror(response));
        result = -EINVAL;
        goto error;
    }

    if (NULL != docker->list.object) {
        enum json_tokener_error error = json_tokener_get_error(
            docker->list.tokener);
        fprintf(stderr, "Couldn't enumerate docker volumes: %s\n",
            json_tokener_error_desc(error));
        result = -EINVAL;
        goto error;
    }

    json_c_visit(docker->list.object, 0, priv_visit_list, docker);

 error:
    json_tokener_free(docker->list.tokener);
    return result;
}

// Currently no use case for these?
int docker_volume_inspect(Docker* proxy, const char* name,
    void (*visitor)(DockerVolume*));
int docker_volume_remove(Docker* proxy, const char* name);

///////////////////////////////////////////////////////////////////////////////
