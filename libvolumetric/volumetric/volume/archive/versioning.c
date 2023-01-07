///////////////////////////////////////////////////////////////////////////////
// NAME:            versioning.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to version archive volumes according to user spec.
//
// CREATED:         01/17/2022
//
// LAST EDITED:     01/07/2023
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

// In this case, if the volume already exists, we do nothing.
int archive_volume_update_policy_never(ArchiveVolume* volume, Docker* docker) {
    int result = docker_volume_exists(docker, volume->name);
    if (result < 0) {
        return result;
    } else if (result == 1) {
        printf("%s: Volume exists, taking no further action.\n", volume->name);
        return VOLUMETRIC_NO_ACTION;
    }

    return VOLUMETRIC_ACTION_REQUIRED;
}

int archive_volume_check_hash(ArchiveVolume* volume, Docker* docker,
                              const FileContents* file) {
    // Hash the contents of the file (in memory) to verify against config
    printf("%s: Checking hash of file %s\n", volume->name, volume->url);
    FileHash* file_hash = file_hash_of_buffer(volume->hash->hash_type,
                                              file->contents, file->size);
    if (!file_hash_equal(volume->hash, file_hash)) {
        char* expected = file_hash_to_string(volume->hash);
        char* got = file_hash_to_string(file_hash);
        file_hash_free(file_hash);
        file_hash = NULL;
        const char* hash_type =
            file_hash_type_to_string(volume->hash->hash_type);
        fprintf(stderr,
                "%s: Error: %s hash mismatch for file %s.\n"
                "Expected:\n"
                "    %s\n"
                "Got:\n"
                "    %s\n",
                volume->name, hash_type, volume->url, expected, got);
        free(expected);
        free(got);
        return -EINVAL;
    }
    file_hash_free(file_hash);
    file_hash = NULL;

    return 0;
}

int archive_volume_update_policy_on_stale_lock(ArchiveVolume* volume,
                                               Docker* docker) {
    // If the lock file does not exist, checkout the volume.
    ArchiveLockFile* lock_file = archive_lock_file_open(volume->name);
    if (NULL == lock_file) {
        return VOLUMETRIC_ACTION_REQUIRED;
    }

    // If the modification time of the volume image is more recent than the
    // modification time of the lock file, updates are required.
    const struct timespec lock_stat = archive_lock_file_get_mtime(lock_file);
    archive_lock_file_close(lock_file);

    struct stat archive_stat = {0};
    // TODO: If ever support for more schemes than plain old absolute paths is
    // added, this will need to be updated to actually parse the URL.
    int result = stat(volume->url, &archive_stat);
    if (0 != result) {
        return result;
    }

    bool lock_stale = archive_stat.st_mtim.tv_sec > lock_stat.tv_sec;
    if (lock_stale) {
        printf("%s: Lock file is stale; performing checkout\n", volume->name);
        return VOLUMETRIC_ACTION_REQUIRED;
    }

    printf("%s: Lock file is up-to-date; taking no further action\n",
           volume->name);
    return VOLUMETRIC_NO_ACTION;
}

int archive_volume_check_remove_existing_volume(ArchiveVolume* volume,
                                                Docker* docker,
                                                const FileContents*) {
    // Remove the volume if it exists, to prevent contamination.
    int result = 0;
    if (docker_volume_exists(docker, volume->name)) {
        result = docker_volume_remove(docker, volume->name);
    }

    return result;
}

int archive_volume_commit_update_lock_file(ArchiveVolume* volume,
                                           Docker* docker) {
    ArchiveLockFile* lock_file = archive_lock_file_create(volume->name);
    if (NULL == lock_file) {
        char message[80] = {0};
        strerror_r(errno, message, sizeof(message));
        fprintf(stderr, "Couldn't create lock file: %s\n", message);
        return -EINVAL;
    }

    archive_lock_file_close(lock_file);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

int archive_volume_checkout(ArchiveVolume* config, Docker* docker) {
    // Apply the update policy to determine whether any action is required.
    int result = config->update_policy(config, docker);
    if (VOLUMETRIC_NO_ACTION == result || 0 > result) {
        return result;
    }

    // Map the file to memory
    FileContents file = {0};
    file_contents_init(&file, config->url);

    // Run a check action to determine that the checkout is safe to perform.
    if (NULL != config->check) {
        result = config->check(config, docker, &file);
        if (0 > result) {
            file_contents_release(&file);
            return result;
        }
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
    file_contents_release(&file);

    docker_volume_free(volume);

    // Run any commit action
    if (NULL != config->commit) {
        result = config->commit(config, docker);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
