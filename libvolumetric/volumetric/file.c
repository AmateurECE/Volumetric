///////////////////////////////////////////////////////////////////////////////
// NAME:            file.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the file interface.
//
// CREATED:         01/21/2022
//
// LAST EDITED:     01/22/2022
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
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <volumetric/file.h>

typedef struct MemoryMappedFile {
    int (*open)(FileContents* file, const char* path);
    void (*close)(FileContents* file);
    union {
        struct {
            int fd;
        } local;
    };
} MemoryMappedFile;

///////////////////////////////////////////////////////////////////////////////
// File Scheme
////

static int file_open(FileContents* file, const char* path)
{
    MemoryMappedFile* map = (MemoryMappedFile*)file->file;
    map->local.fd = open(path, O_RDONLY);
    assert(-1 != map->local.fd);

    struct stat file_stats = {0};
    assert(0 == fstat(map->local.fd, &file_stats));

    file->contents = mmap(NULL, file_stats.st_size, PROT_READ, MAP_SHARED,
        map->local.fd, 0);
    assert(MAP_FAILED != file->contents);
    file->size = file_stats.st_size;
    return 0;
}

static void file_close(FileContents* file)
{ munmap(file->contents, file->size); }

static bool is_file_schema(const char* path) {
    static const char* file_scheme = "file://";
    char* colon = strchr(path, ':');
    if (NULL == colon || '/' == *path) {
        return true;
    } else if (!strncmp(file_scheme, path, strlen(file_scheme))) {
        return true;
    }

    return false;
}

static void init_callbacks_for_file_scheme(FileContents* file,
    const char* path)
{ file->file->open = file_open; file->file->close = file_close; }

///////////////////////////////////////////////////////////////////////////////
// Private Functions
////

static int init_callbacks_for_scheme(FileContents* file, const char* path) {
    // Currently, only the file scheme is supported.
    if (is_file_schema(path)) {
        init_callbacks_for_file_scheme(file, path);
    } else {
        fprintf(stderr, "%s:%d: Unknown schema for path: %s\n", __FILE__,
            __LINE__, path);
        return -EINVAL;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

// Open the file at <path> and fill the memory buffer with its contents
int file_contents_init(FileContents* file, const char* path) {
    memset(file, 0, sizeof(FileContents));
    file->file = malloc(sizeof(MemoryMappedFile));
    assert(NULL != file->file);
    assert(0 == init_callbacks_for_scheme(file, path));

    file->file->open(file, path);
    return 0;
}

// Release memory held by this object.
void file_contents_release(FileContents* file)
{ file->file->close(file); free(file->file); }

///////////////////////////////////////////////////////////////////////////////
