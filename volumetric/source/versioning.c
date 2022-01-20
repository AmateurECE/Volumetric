///////////////////////////////////////////////////////////////////////////////
// NAME:            versioning.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to version the files according to user spec.
//
// CREATED:         01/17/2022
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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include <glib-2.0/glib.h>
#include <gobiserde/yaml.h>

#include <docker.h>
#include <versioning.h>
#include <volume.h>

struct VolumeFilter {
    const char* name;
    bool found;
};

static int filter_volume_by_name(const DockerVolume* volume, void* user_data)
{
    struct VolumeFilter* filter = (struct VolumeFilter*)user_data;
    if (!strcmp(filter->name, volume->name)) {
        filter->found = true;
        return DOCKER_VISITOR_STOP;
    }

    return DOCKER_VISITOR_CONTINUE;
}

static void version_archive_volume(ArchiveVolume* volume, Docker* docker)
{
    struct VolumeFilter filter = {0};
    filter.name = volume->name;
    docker_volume_list(docker, filter_volume_by_name, &filter);
    // TODO: The rest of this.
}

static void version_volume(void* key __attribute__((unused)), void* value,
    void* user_data)
{
    Volume* volume = (Volume*)value;
    Docker* docker = (Docker*)user_data;
    switch (volume->type) {
    case VOLUME_TYPE_ARCHIVE:
        version_archive_volume(&volume->archive, docker);
        break;
    default: break;
    }
}

int version_volumes_in_file(FILE* input)
{
    VolumeFile volume_file = {0};
    yaml_deserializer* yaml = gobiserde_yaml_deserializer_new_file(input);
    int result = volume_file_deserialize_from_yaml(yaml, &volume_file);
    gobiserde_yaml_deserializer_free(&yaml);

    if (0 == result) {
        Docker* docker = docker_proxy_new();
        g_hash_table_foreach(volume_file.volumes, version_volume, docker);
        docker_proxy_free(docker);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////