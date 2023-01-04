///////////////////////////////////////////////////////////////////////////////
// NAME:            internal.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Methods internal to the Docker proxy interface.
//
// CREATED:         02/13/2022
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

#ifndef VOLUMETRIC_DOCKER_INTERNAL_H
#define VOLUMETRIC_DOCKER_INTERNAL_H

#include <volumetric/docker.h>

// NOTE: THESE METHODS ARE INTERNAL TO THE DOCKER PROXY API AND SHOULD NOT
// BE USED EXCEPT BY FILES IN THIS DIRECTORY

// Perform chunked transfer encoding on a JSON object
int http_encode(json_object* object, char** string, size_t* length);

// GET an application/json object
int http_get_application_json(Docker* docker, const char* url);

// POST with an application/json request object
int http_post_application_json(Docker* docker, const char* url);

// POST without a request object
int http_post(Docker* docker, const char* url);

// DELETE without a request object
int http_delete(Docker* docker, const char* url);

#endif // VOLUMETRIC_DOCKER_INTERNAL_H

///////////////////////////////////////////////////////////////////////////////
