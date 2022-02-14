///////////////////////////////////////////////////////////////////////////////
// NAME:            internal.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Methods internal to the Docker proxy interface.
//
// CREATED:         02/13/2022
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

#ifndef VOLUMETRIC_DOCKER_INTERNAL_H
#define VOLUMETRIC_DOCKER_INTERNAL_H

#include <volumetric/docker.h>

// NOTE: THESE METHODS ARE INTERNAL TO THE DOCKER PROXY API AND SHOULD NOT
// BE USED EXCEPT BY FILES IN THIS DIRECTORY

int http_encode(json_object* object, char** string, size_t* length);
size_t copy_data_to_curl_request(char* buffer,
    size_t size __attribute__((unused)), size_t nitems, void* user_data);
size_t copy_data_from_curl_response(void* buffer,
    size_t size __attribute__((unused)), size_t nmemb, void* user_data);
int http_get_application_json(Docker* docker, const char* url);
int http_post_application_json(Docker* docker, const char* url);

#endif // VOLUMETRIC_DOCKER_INTERNAL_H

///////////////////////////////////////////////////////////////////////////////
