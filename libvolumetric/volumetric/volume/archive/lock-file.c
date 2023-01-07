///////////////////////////////////////////////////////////////////////////////
// NAME:            lock-file.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the simple lock file interface.
//
// CREATED:         12/30/2022
//
// LAST EDITED:	    01/07/2023
//
////
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lock-file.h"

static const char* VOLUMETRIC_LOCK_DIRECTORY = "/var/local/volumetric";
static const char* VOLUMETRIC_LOCK_EXTENSION = ".lock";

typedef struct ArchiveLockFile {
    int fd;
    char* path;
} ArchiveLockFile;

static ArchiveLockFile* archive_lock_file_new() {
    ArchiveLockFile* lock_file = malloc(sizeof(ArchiveLockFile));
    memset(lock_file, 0, sizeof(*lock_file));
    return lock_file;
}

static char* archive_lock_file_get_path(const char* volume_name) {
    size_t length = strlen(VOLUMETRIC_LOCK_DIRECTORY) + 1 +
                    strlen(volume_name) + strlen(VOLUMETRIC_LOCK_EXTENSION) +
                    1;
    char* path = malloc(length);
    if (NULL == path) {
        return NULL;
    }

    memset(path, 0, length);
    strcat(path, VOLUMETRIC_LOCK_DIRECTORY);
    strcat(path, "/");
    strcat(path, volume_name);
    strcat(path, VOLUMETRIC_LOCK_EXTENSION);
    return path;
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

ArchiveLockFile* archive_lock_file_create(const char* volume_name) {
    ArchiveLockFile* lock_file = archive_lock_file_new();
    lock_file->path = archive_lock_file_get_path(volume_name);

    lock_file->fd = open(lock_file->path, O_CREAT | O_TRUNC | O_RDWR);
    if (0 > lock_file->fd) {
        free(lock_file);
        return NULL;
    }

    return lock_file;
}

ArchiveLockFile* archive_lock_file_open(const char* volume_name) {
    ArchiveLockFile* lock_file = archive_lock_file_new();
    lock_file->path = archive_lock_file_get_path(volume_name);
    if (NULL == lock_file->path) {
        free(lock_file);
        return NULL;
    }

    lock_file->fd = open(lock_file->path, O_RDONLY);
    if (0 > lock_file->fd) {
        free(lock_file->path);
        free(lock_file);
        return NULL;
    }

    return lock_file;
}

void archive_lock_file_close(ArchiveLockFile* file) {
    close(file->fd);
    free(file->path);
    free(file);
}

struct timespec archive_lock_file_get_mtime(ArchiveLockFile* file) {
    struct stat file_stat = {0};
    int result = fstat(file->fd, &file_stat);
    if (0 != result) {
        char message[80] = {0};
        strerror_r(errno, message, sizeof(message));
        fprintf(stderr, "Couldn't stat %s: %s\n", file->path, message);
        return (struct timespec){0};
    }

    return file_stat.st_mtim;
}

///////////////////////////////////////////////////////////////////////////////
