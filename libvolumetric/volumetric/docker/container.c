///////////////////////////////////////////////////////////////////////////////
// NAME:            container.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for working with Docker containers.
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

#include <stdlib.h>

#include <volumetric/docker.h>

///////////////////////////////////////////////////////////////////////////////
// Public API
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
