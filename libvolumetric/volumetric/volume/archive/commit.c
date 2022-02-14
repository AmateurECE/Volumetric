///////////////////////////////////////////////////////////////////////////////
// NAME:            commit.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to commit archive volumes to the source.
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

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glib-2.0/glib.h>

#include <volumetric/docker.h>
#include <volumetric/string-handling.h>
#include <volumetric/volume/archive.h>

///////////////////////////////////////////////////////////////////////////////
// Filename Stuff
////

static char* get_dirname_owned(const char* filename) {
    char* filename_owned = strdup(filename);
    char* dirname_owned = strdup(dirname(filename_owned));
    free(filename_owned);
    return dirname_owned;
}

static char* get_basename_owned(const char* filename) {
    char* filename_owned = strdup(filename);
    char* basename_owned = strdup(basename(filename_owned));
    free(filename_owned);
    return basename_owned;
}

static char* get_extension_owned(const char* filename) {
    char* basename_owned = get_basename_owned(filename);
    char* extension_start = strchr(basename_owned, '.');
    if (extension_start == NULL) {
        free(basename_owned);
        return strdup("");
    }

    char* extension_owned = strdup(extension_start);
    free(basename_owned);
    return extension_owned;
}

static char* get_basename_without_extension_owned(const char* filename) {
    char* basename_owned = get_basename_owned(filename);
    char* extension_start = strchr(basename_owned, '.');
    if (NULL == extension_start) {
        return basename_owned;
    }

    *extension_start = '\0';
    char* basename_without_extension_owned = strdup(basename_owned);
    free(basename_owned);
    return basename_without_extension_owned;
}

static char* get_new_filename(const char* current_url, const char* suffix) {
    char* dirname_owned = get_dirname_owned(current_url);
    char* basename_without_extension_owned =
        get_basename_without_extension_owned(current_url);
    char* extension_owned = get_extension_owned(current_url);

    char* new_filename = string_join_new(dirname_owned, '/',
        basename_without_extension_owned);
    free(basename_without_extension_owned);
    new_filename = string_join_new(new_filename, '-', suffix);
    new_filename = string_append_new(new_filename, extension_owned);
    free(extension_owned);

    return new_filename;
}

///////////////////////////////////////////////////////////////////////////////
// Other Private Functions
////

static char* get_date_string_owned() {
    struct timespec current_time = {0};
    // Get the clock time for the monotonic clock
    int result = clock_gettime(CLOCK_REALTIME_COARSE, &current_time);
    if (0 != result) {
        perror("couldn't get system time");
        return NULL;
    }

    // Break it down into a struct tm (needed for string conversion)
    struct tm time_parts = {0};
    localtime_r(&current_time.tv_sec, &time_parts);

    size_t string_length = 64;
    char* string = malloc(string_length);
    if (NULL == string) {
        perror("couldn't get system time");
        return NULL;
    }

    // Convert it into a string
    memset(string, 0, string_length);
    size_t bytes_written = strftime(string, string_length - 1,
        "%Y%m%d-%H%M%S", &time_parts);
    if (0 == bytes_written) {
        perror("couldn't get system time");
        free(string);
        return NULL;
    }

    return string;
}

static GPtrArray* get_consumers_of_volume(Docker* docker,
    const char* volume_name)
{
    GPtrArray* consumers = g_ptr_array_new();

    DockerContainerIter* containers = docker_container_list(docker);
    const DockerContainer* container = NULL;
    while (NULL != (container = docker_container_iter_next(containers))) {
        const DockerMount* mount = NULL;
        while (NULL != (mount = docker_mount_iter_next(container->mounts))) {
            if (!strcmp(mount->source, volume_name)) {
                g_ptr_array_add(consumers, strdup(container->name));
            }
        }
    }
    return consumers;
}

static int commit_changes(const char* mountpoint, const char* filename)
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

int archive_volume_commit(ArchiveVolume* volume, Docker* docker, bool dry_run)
{
    // Rename the current source file to save it.
    char* current_time = get_date_string_owned();
    char* new_filename = get_new_filename(volume->url, current_time);
    free(current_time);

    printf("%s: Renaming %s to %s\n", volume->name, volume->url, new_filename);
    if (dry_run) {
        return 0; // Basically nothing else we can do if we're dry-running.
    }

    int result = rename(volume->url, new_filename);
    free(new_filename);
    if (0 != result) {
        perror("couldn't rename source");
        return -1 * errno;
    }

    // Get the list of containers that have this volume mounted
    GPtrArray* containers = get_consumers_of_volume(docker, volume->name);

    // Pause any running containers that have the volume mounted
    printf("Pausing any containers that have this volume mounted...\n");
    for (guint i = 0; i < containers->len; ++i) {
        printf("Pausing %s\n", (const char*)containers->pdata[i]);
        result = docker_container_pause(docker,
            (const char*)containers->pdata[i]);
        if (0 != result) {
            g_ptr_array_unref(containers);
            return result;
        }
    }

    // Commit changes to disk
    DockerVolume* live_volume = docker_volume_inspect(docker, volume->name);
    result = commit_changes(live_volume->mountpoint, volume->url);

    // Un-pause all the containers that have the volume mounted
    for (guint i = 0; i < containers->len; ++i) {
        printf("Un-pausing %s\n", (const char*)containers->pdata[i]);
        result = docker_container_unpause(docker,
            (const char*)containers->pdata[i]);
        if (0 != result) {
            g_ptr_array_unref(containers);
            return result;
        }
    }

    g_ptr_array_unref(containers);
    return -ENOSYS;
}

///////////////////////////////////////////////////////////////////////////////
