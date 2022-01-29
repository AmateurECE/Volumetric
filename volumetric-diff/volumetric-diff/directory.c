///////////////////////////////////////////////////////////////////////////////
// NAME:            directory.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic implementing the directory iterator interface.
//
// CREATED:         01/29/2022
//
// LAST EDITED:     01/29/2022
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <volumetric-diff/directory.h>

typedef struct DirectoryIter {
    char* directory_owned;
    size_t directory_owned_length;
    char* path_buffer;
    DIR* directory;
    DirectoryEntry entry;
} DirectoryIter;

static const size_t ABSOLUTE_PATH_CAPACITY = PATH_MAX;

DirectoryIter* directory_iter_new(const char* directory) {
    DIR* system_directory = opendir(directory);
    if (NULL == system_directory) {
        fprintf(stderr, "%s:%d: Couldn't open directory: %s (%s)\n", __FILE__,
            __LINE__, directory, strerror(errno));
    }

    DirectoryIter* iter = malloc(sizeof(DirectoryIter));
    assert(NULL != iter);
    memset(iter, 0, sizeof(DirectoryIter));

    iter->directory = system_directory;
    iter->directory_owned = strdup(directory);
    iter->directory_owned_length = strlen(directory);
    iter->path_buffer = malloc(ABSOLUTE_PATH_CAPACITY);
    memset(iter->path_buffer, 0, ABSOLUTE_PATH_CAPACITY);
    return iter;
}

void directory_iter_free(DirectoryIter* iter) {
    free(iter->directory_owned);
    free(iter->path_buffer);
    closedir(iter->directory);
    free(iter);
}

DirectoryEntry* directory_iter_next(DirectoryIter* iter) {
    bool entry_found = false;
    struct dirent* entry = NULL;
    while (!entry_found) {
        entry = readdir(iter->directory);
        if (NULL == entry) {
            return NULL;
        }

        // Ignore "." and ".."
        entry_found = strcmp(".", entry->d_name)
            && strcmp("..", entry->d_name);
    }

    iter->entry.entry = entry;
    memset(iter->path_buffer, 0, ABSOLUTE_PATH_CAPACITY);
    strcpy(iter->path_buffer, iter->directory_owned);
    iter->path_buffer[iter->directory_owned_length] = '/';
    strcpy(iter->path_buffer + iter->directory_owned_length + 1,
        entry->d_name);
    iter->entry.absolute_path = iter->path_buffer;
    return &iter->entry;
}

///////////////////////////////////////////////////////////////////////////////
