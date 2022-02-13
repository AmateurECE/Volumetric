///////////////////////////////////////////////////////////////////////////////
// NAME:            status.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic around querying status of archive volumes.
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

#include <assert.h>
#include <errno.h>
#include <fts.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>
#include <glib-2.0/glib.h>

#include <volumetric/directory.h>
#include <volumetric/docker.h>
#include <volumetric/file.h>
#include <volumetric/string-handling.h>
#include <volumetric/volume/archive.h>

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static bool check_file_for_modifications(struct archive_entry* entry,
    const char* directory_file)
{
    // Check for differences based on stat data
    struct stat file_stat = {0};
    assert(0 == stat(directory_file, &file_stat));

    const struct stat* archive_stat = archive_entry_stat(entry);
    bool diff = false;
    if (S_ISREG(file_stat.st_mode)) {
        diff = diff || file_stat.st_size != archive_stat->st_size;
    }

    diff = diff || file_stat.st_mode != archive_stat->st_mode;
    diff = diff || file_stat.st_mtime != archive_stat->st_mtime;
    return diff;
}

// Find what's changed in the directory from the archive
static int diff_directory_from_archive(GPtrArray* directory,
    const char* archive_url, const char* directory_base)
{
    struct archive *reader = archive_read_new();
    struct archive_entry *entry = NULL;
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    FileContents archive = {0};
    file_contents_init(&archive, archive_url);
    archive_read_open_memory(reader, archive.contents, archive.size);
    while (archive_read_next_header(reader, &entry) == ARCHIVE_OK) {
        static const char* archive_base = "./";
        const char* entry_path = archive_entry_pathname(entry);
        if (!strcmp(archive_base, entry_path)) {
            // Skip "./"
            continue;
        }

        char* archive_file = string_new(entry_path + strlen(archive_base));
        size_t archive_file_length = strlen(archive_file);
        if ('/' == archive_file[archive_file_length - 1]) {
            // Truncate trailing '/'
            archive_file[archive_file_length - 1] = '\0';
        }

        bool found = false;
        for (guint i = 0; i < directory->len; ++i) {
            const char* directory_file = directory->pdata[i];
            if (!strcmp(archive_file, directory_file)) {
                found = true;
                char* full_path = string_append_new(string_new(directory_base),
                    directory_file);
                if (check_file_for_modifications(entry, full_path)) {
                    printf("M %s\n", archive_file);
                }
                g_ptr_array_remove_index_fast(directory, i);
                break;
            }
        }

        if (!found) {
            printf("D %s\n", (const char*)archive_file);
        }
        free(archive_file);
    }

    for (guint i = 0; i < directory->len && NULL != directory->pdata[i]; ++i) {
        printf("A %s\n", (const char*)directory->pdata[i]);
    }

    archive_read_free(reader);
    file_contents_release(&archive);
    return 0;
}

static GPtrArray* get_file_list_for_directory(const char* directory) {
    GPtrArray* list = g_ptr_array_new_with_free_func(free);

    char* directory_owned = string_new(directory);
    char* const paths[] = {directory_owned, NULL};
    FTS* tree = fts_open(paths, FTS_NOCHDIR, 0);
    assert(NULL != tree);

    // TODO: How does this work with symlinks? I'd expect they would break.
    FTSENT* node = NULL;
    while ((node = fts_read(tree))) {
        if (FTS_F == node->fts_info || FTS_D == node->fts_info) {
            g_ptr_array_add(list, strdup(node->fts_path));
        } else if (FTS_ERR == node->fts_info || FTS_DNR == node->fts_info
            || FTS_NS == node->fts_info) {
            fprintf(stderr, "fts_read error: %s\n", strerror(node->fts_errno));
            exit(errno);
        }
    }

    fts_close(tree);
    free(directory_owned);
    return list;
}

static void remove_matching_entry(GPtrArray* list, const char* match) {
    // First, remove the entry corresponding to the match
    guint index = 0;
    for (guint i = 0; i < list->len && NULL != list->pdata[i]; ++i) {
        if (!strcmp(match, list->pdata[i])) {
            index = i;
            break;
        }
    }
    g_ptr_array_remove_index_fast(list, index);
}

static void trim_prefix_from_entries(GPtrArray* list, const char* prefix) {
    // Then, remove the prefix from any entries that have it
    size_t prefix_length = strlen(prefix);
    for (guint i = 0; i < list->len && NULL != list->pdata[i]; ++i) {
        if (!strncmp(prefix, list->pdata[i], prefix_length)) {
            char* string = string_new(list->pdata[i] + prefix_length);
            free(list->pdata[i]);
            list->pdata[i] = string;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

int archive_volume_diff(ArchiveVolume* volume, Docker* docker) {
    DockerVolume* live_volume = docker_volume_inspect(docker, volume->name);
    docker_proxy_free(docker);
    assert(NULL != live_volume);

    GPtrArray* directory = get_file_list_for_directory(
        live_volume->mountpoint);

    // First, let's remove the top-level entry (either "./" or "/...")
    remove_matching_entry(directory, live_volume->mountpoint);
    size_t mountpoint_length = strlen(live_volume->mountpoint);
    char* mountpoint = strdup(live_volume->mountpoint);
    if ('/' != live_volume->mountpoint[mountpoint_length - 1]) {
        // Have to add that terminating '/'
        mountpoint = string_append_new(string_new(live_volume->mountpoint),
            "/");
    }
    trim_prefix_from_entries(directory, mountpoint);

    int result = diff_directory_from_archive(directory, volume->url,
        mountpoint);
    free(mountpoint);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
