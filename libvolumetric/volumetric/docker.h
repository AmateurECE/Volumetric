///////////////////////////////////////////////////////////////////////////////
// NAME:            docker.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Proxy interface to the Docker daemon (using socket)
//
// CREATED:         01/17/2022
//
// LAST EDITED:     01/02/2023
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

#ifndef VOLUMETRIC_DOCKER_H
#define VOLUMETRIC_DOCKER_H

#include <stddef.h>

typedef void CURL;
typedef struct json_tokener json_tokener;
typedef struct json_object json_object;

typedef struct DockerVolumeListIter DockerVolumeListIter;
typedef struct DockerMountIter DockerMountIter;
typedef struct DockerContainerIter DockerContainerIter;

///////////////////////////////////////////////////////////////////////////////
// Docker Proxy General API
////

typedef struct Docker {
    CURL* curl;

    json_tokener* tokener;
    json_object* write_object;
    char* read_object;
    size_t read_object_length;
    size_t read_object_index;

} Docker;

Docker* docker_proxy_new();
void docker_proxy_free(Docker* docker);

///////////////////////////////////////////////////////////////////////////////
// Docker Volume API
////

typedef struct DockerVolume {
    char* name;
    char* driver;
    char* mountpoint;
} DockerVolume;

// Create the volume
DockerVolume* docker_volume_create(Docker* docker, const char* name);

// Iteration API for the /volumes endpoint
DockerVolumeListIter* docker_volume_list(Docker* docker);
const DockerVolume* docker_volume_list_iter_next(DockerVolumeListIter* iter);
void docker_volume_list_iter_free(DockerVolumeListIter* iter);

// Release internal memory held by the DockerVolume instance.
void docker_volume_free(DockerVolume* volume);

// Get all the attributes for the Docker volume with the given name.
DockerVolume* docker_volume_inspect(Docker* docker, const char* name);

// Remove a volume from the system
int docker_volume_remove(Docker* docker, const char* name);

// Returns:
//  0: Volume does not exist
//  1: Volume exists
//  <0: Error code
int docker_volume_exists(Docker* docker, const char* name);

///////////////////////////////////////////////////////////////////////////////
// Docker Container API
////

typedef struct DockerMount {
    char* source;
} DockerMount;

typedef struct DockerContainer {
    char* id;
    DockerMountIter* mounts;
} DockerContainer;

// List the containers the engine is managing
DockerContainerIter* docker_container_list(Docker* docker);

// Mutate the iterator
const DockerContainer* docker_container_iter_next(DockerContainerIter* iter);
void docker_container_iter_free(DockerContainerIter* iter);

// Iterate through mounts
const DockerMount* docker_mount_iter_next(DockerMountIter* iter);
void docker_mount_iter_free(DockerMountIter* iter);

// Pause/Un-pause a container
int docker_container_pause(Docker* docker, const char* container_id);
int docker_container_unpause(Docker* docker, const char* container_id);

// Free memory
void docker_container_free(DockerContainer* container);
void docker_mount_free(DockerMount* mount);

#endif // VOLUMETRIC_DOCKER_H

///////////////////////////////////////////////////////////////////////////////
