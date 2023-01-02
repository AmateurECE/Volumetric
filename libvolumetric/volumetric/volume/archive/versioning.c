///////////////////////////////////////////////////////////////////////////////
// NAME:            versioning.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to version archive volumes according to user spec.
//
// CREATED:         01/17/2022
//
// LAST EDITED:     01/01/2023
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
#include <sys/stat.h>

#include <glib-2.0/glib.h>
#include <serdec/yaml.h>

#include <volumetric/archive.h>
#include <volumetric/docker.h>
#include <volumetric/file.h>
#include <volumetric/hash.h>
#include <volumetric/volume/archive.h>
#include <volumetric/volume/archive/lock-file.h>

enum {
    NO_ACTION = 0,
    ACTION_REQUIRED = 1,
};

static int archive_checkout_is_necessary(ArchiveVolume* config,
                                         Docker* docker) {
    // First, check to make sure the volume doesn't already exist. If the
    // configuration does not ask us to maintain a lock file, and the volume
    // already exists, no action is necessary.
    int result = docker_volume_exists(docker, config->name);
    if (result < 0) {
        return result;
    } else if (result == 1 && !config->update_on_stale_lock) {
        printf("%s: Volume exists, taking no further action.\n", config->name);
        return NO_ACTION;
    }

    // If the lock file does not exist, checkout the volume.
    ArchiveLockFile* lock_file = archive_lock_file_open(config->name);
    if (NULL == lock_file) {
        return ACTION_REQUIRED;
    }

    // If the hash in the lock file is different from the hash in the
    // configuration and the modification time of the volume image is more
    // recent than the modification time of the lock file, updates are
    // required.
    const FileHash* locked_hash = archive_lock_file_get_hash(lock_file);
    bool hash_equal = file_hash_equal(locked_hash, config->hash);

    const struct timespec lock_stat = archive_lock_file_get_mtime(lock_file);
    archive_lock_file_close(lock_file);

    struct stat archive_stat = {0};
    // TODO: If ever support for more schemes than plain old absolute paths is
    // added, this will need to be updated to actually parse the URL.
    result = stat(config->url, &archive_stat);
    if (0 != result) {
        return result;
    }

    bool lock_stale = archive_stat.st_mtim.tv_sec <= lock_stat.tv_sec;
    if (!hash_equal && lock_stale) {
        printf("%s: Lock file is stale; performing checkout\n", config->name);
        return ACTION_REQUIRED;
    }

    return NO_ACTION;
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

int archive_volume_checkout(ArchiveVolume* config, Docker* docker) {
    int result = archive_checkout_is_necessary(config, docker);
    if (0 == result || 0 > result) {
        return result;
    }

    // Map the file to memory
    FileContents file = {0};
    file_contents_init(&file, config->url);

    // Hash the contents of the file (in memory) to verify against config
    printf("%s: Checking hash of file %s\n", config->name, config->url);
    FileHash* file_hash =
        file_hash_of_buffer(config->hash->hash_type, file.contents, file.size);
    if (!file_hash_equal(config->hash, file_hash)) {
        char* expected = file_hash_to_string(config->hash);
        char* got = file_hash_to_string(file_hash);
        file_hash_free(file_hash);
        file_hash = NULL;
        const char* hash_type =
            file_hash_type_to_string(config->hash->hash_type);
        fprintf(stderr,
                "%s: Error: %s hash mismatch for file %s.\n"
                "Expected:\n"
                "    %s\n"
                "Got:\n"
                "    %s\n",
                config->name, hash_type, config->url, expected, got);
        file_contents_release(&file);
        free(expected);
        free(got);
        return -EINVAL;
    }
    file_hash_free(file_hash);
    file_hash = NULL;

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
    file_contents_release(&file);

    docker_volume_free(volume);

    // If the config wants us to keep a lock file, update that now.
    if (config->update_on_stale_lock) {
        ArchiveLockFile* lock_file = archive_lock_file_create(config->name);
        archive_lock_file_update(lock_file, config->hash);
        archive_lock_file_close(lock_file);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
