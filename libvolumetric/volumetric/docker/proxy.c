///////////////////////////////////////////////////////////////////////////////
// NAME:            proxy.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     General docker proxy routines
//
// CREATED:         02/13/2022
//
// LAST EDITED:     06/23/2022
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

#include <errno.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>

#include <volumetric/docker.h>
#include <volumetric/docker/internal.h>

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static const char* DOCKER_SOCK_PATH = "/var/run/docker.sock";

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

static int http_read_application_json(Docker* docker, const char* url) {
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

///////////////////////////////////////////////////////////////////////////////
// Internal API
////

int http_encode(json_object* object, char** string, size_t* length) {
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

int http_get_application_json(Docker* docker, const char* url) {
    curl_easy_setopt(docker->curl, CURLOPT_HTTPGET, 1);
    return http_read_application_json(docker, url);
}

int http_post_application_json(Docker* docker, const char* url) {
    curl_easy_setopt(docker->curl, CURLOPT_READFUNCTION,
        copy_data_to_curl_request);
    curl_easy_setopt(docker->curl, CURLOPT_READDATA, docker);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(docker->curl, CURLOPT_HTTPHEADER, headers);
    return http_post(docker, url);
}

int http_post(Docker* docker, const char* url) {
    curl_easy_setopt(docker->curl, CURLOPT_POST, 1);
    return http_read_application_json(docker, url);
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

    // Convention allows the user to override the path to the docker socket
    // with the DOCKER_HOST environment variable.
    char* docker_host = getenv("DOCKER_HOST");
    if (NULL != docker_host) {
        const char* unix = "unix";
        if (strncmp(docker_host, unix, strlen(unix))) {
            fprintf(stderr, "Unknown url scheme in DOCKER_HOST '%s'",
                docker_host);
            free(docker);
            return NULL;
        }

        docker_host += strlen(unix) + strlen(":/");
        docker->curl = curl_easy_init();
        curl_easy_setopt(docker->curl, CURLOPT_UNIX_SOCKET_PATH, docker_host);
    } else {
        docker->curl = curl_easy_init();
        curl_easy_setopt(docker->curl, CURLOPT_UNIX_SOCKET_PATH,
            DOCKER_SOCK_PATH);
    }

    return docker;
}

void docker_proxy_free(Docker* docker) {
    curl_easy_cleanup(docker->curl);
    free(docker);
}

///////////////////////////////////////////////////////////////////////////////
