///////////////////////////////////////////////////////////////////////////////
// NAME:            versioning.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to version archive volumes according to user spec.
//
// CREATED:         01/17/2022
//
// LAST EDITED:     02/15/2022
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
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include <glib-2.0/glib.h>
#include <serdec/yaml.h>

#include <volumetric/archive.h>
#include <volumetric/docker.h>
#include <volumetric/file.h>
#include <volumetric/hash.h>
#include <volumetric/volume/archive.h>

int archive_volume_checkout(ArchiveVolume* config, Docker* docker) {
    // First, check to make sure the volume doesn't already exist
    DockerVolumeListIter* iter = docker_volume_list(docker);
    const DockerVolume* list_entry = NULL;
    bool action_necessary = true;
    while (NULL != (list_entry = docker_volume_list_iter_next(iter))) {
        if (!strcmp(config->name, list_entry->name)) {
            printf("%s: Volume exists, taking no further action.\n",
                config->name);
            action_necessary = false;
            break;
        }
    }
    docker_volume_list_iter_free(iter);
    if (!action_necessary) {
        return 0;
    }

    // Map the file to memory
    FileContents file = {0};
    file_contents_init(&file, config->url);

    // Hash the contents of the file (in memory) to verify against config
    printf("%s: Checking hash of file %s\n", config->name, config->url);
    if (!check_hash_of_memory(file.contents, file.size, config->hash))
    {
        fprintf(stderr, "%s: Error: Hash mismatch for file %s\n", config->name,
            config->url);
        file_contents_release(&file);
        return -EINVAL;
    }

    // Create the volume
    printf("%s: Initializing Docker volume\n", config->name);
    DockerVolume* volume = docker_volume_create(docker, config->name);
    if (NULL == volume) {
        file_contents_release(&file);
        return -1 * errno;
    }

    // Decompress it to disk.
    printf("%s: Extracting volume archive image to disk\n", config->name);
    archive_extract_to_disk_universal(&file, volume->mountpoint);

    docker_volume_free(volume);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
