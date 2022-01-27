///////////////////////////////////////////////////////////////////////////////
// NAME:            docker.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Proxy interface to the Docker daemon (using socket)
//
// CREATED:         01/17/2022
//
// LAST EDITED:     01/27/2022
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

typedef struct Docker Docker;
typedef struct DockerListIterator DockerListIterator;

Docker* docker_proxy_new();
void docker_proxy_free(Docker* docker);

typedef struct DockerVolume {
    char* name;
    char* driver;
    char* mountpoint;
} DockerVolume;

enum {
    DOCKER_VISITOR_CONTINUE=0,
    DOCKER_VISITOR_STOP=1,
};

// TODO: I hate this interface. It would be nice to somehow separate the logic
// of volume operations from the proxy interface. Like some kind of a:
//
//  DockerVolumeResult result = docker_volume_new(name);
//  docker_volume_defaults(&volume);
//  volume.member = foo;
//  ...
//  DockerOperation* operation = docker_operation_volume_create(proxy, volume);
//  docker_operation_complete(operation);
//
// Perhaps, then:
//
//  DockerOperation* op = docker_operation_volume_list(proxy, visitorfn, data);
//  docker_operation_complete(op);
//
// The issue is just that the information needed to complete operations on the
// Docker daemon is not the same information needed to export a useful data
// interface.

// Create the volume
DockerVolume* docker_volume_create(Docker* docker, const char* name);

// The visitor method should return one of the VISITOR_* constants above.
int docker_volume_list(Docker* docker,
    int (*visitor)(const DockerVolume*, void*), void* user_data);

// Release internal memory held by the DockerVolume instance.
void docker_volume_free(DockerVolume* volume);

// Get all the attributes for the Docker volume with the given name.
DockerVolume* docker_volume_inspect(Docker* docker, const char* name);

// This method currently doesn't have an implementation.
int docker_volume_remove(Docker* docker, const char* name);

#endif // VOLUMETRIC_DOCKER_H

///////////////////////////////////////////////////////////////////////////////
