///////////////////////////////////////////////////////////////////////////////
// NAME:            lock-file.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the simple lock file interface.
//
// CREATED:         12/30/2022
//
// LAST EDITED:	    01/01/2023
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

static const char* VOLUMETRIC_LOCK_DIRECTORY = "/var/volumetric";
static const char* VOLUMETRIC_LOCK_EXTENSION = ".lock";

typedef struct ArchiveLockFile {
    int fd;
    char* path;
    FileHash* hash;
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

// An example of a valid lock file is given below: The hash type is followed
// by a colon, and then the ASCII representation of the hash value.
//  md5:abcdef1234567890
static int archive_lock_file_read_hash(ArchiveLockFile* lock_file) {
    // Have to read the whole contents of the file into a string:
    off_t end = lseek(lock_file->fd, 0, SEEK_END);
    off_t start = lseek(lock_file->fd, 0, SEEK_SET);
    size_t length = end - start;
    char* contents = malloc(length + 1);
    memset(contents, 0, length + 1);
    ssize_t result = read(lock_file->fd, contents, length);
    if (length != result) {
        return -1 * errno;
    }

    // Next, find the ':' character.
    char* colon = strchr(contents, ':');
    if (NULL == colon) {
        fprintf(stderr, "Format of lock file %s is incorrect!\n",
                lock_file->path);
        free(contents);
        return -EINVAL;
    }

    *colon = '\0';
    char* value = colon + 1;

    // Convert the contents into a FileHash instance
    FileHashType hash_type = file_hash_type_from_string(contents);
    if (FILE_HASH_TYPE_INVALID == hash_type) {
        fprintf(stderr, "Invalid hash type %s in lock file %s\n", contents,
                lock_file->path);
        free(contents);
        return -EINVAL;
    }

    lock_file->hash = file_hash_from_string(hash_type, value);
    free(contents);
    if (NULL == lock_file->hash) {
        fprintf(stderr, "Lock file %s contains invalid hash value\n",
                lock_file->path);
        return -EINVAL;
    }

    return 0;
}

static int archive_lock_file_write_hash(ArchiveLockFile* lock_file,
                                        const FileHash* hash) {
    const char* hash_type =
        file_hash_type_to_string(lock_file->hash->hash_type);
    ssize_t result = write(lock_file->fd, hash_type, strlen(hash_type));
    if (-1 == result) {
        return -1 * errno;
    }

    static const char* COLON = ":";
    result = write(lock_file->fd, COLON, strlen(COLON));
    if (-1 == result) {
        return -1 * errno;
    }

    char* value = file_hash_to_string(lock_file->hash);
    result = write(lock_file->fd, value, strlen(value));
    free(value);
    if (-1 == result) {
        return -1 * errno;
    }

    return 0;
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
        free(lock_file);
        return NULL;
    }

    int result = archive_lock_file_read_hash(lock_file);
    if (0 != result) {
        close(lock_file->fd);
        free(lock_file);
        return NULL;
    }

    return lock_file;
}

void archive_lock_file_close(ArchiveLockFile* file) {
    close(file->fd);
    file_hash_free(file->hash);
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

const FileHash* archive_lock_file_get_hash(ArchiveLockFile* file) {
    return file->hash;
}

int archive_lock_file_update(ArchiveLockFile* file, const FileHash* hash) {
    archive_lock_file_write_hash(file, hash);
    return archive_lock_file_read_hash(file);
}

///////////////////////////////////////////////////////////////////////////////
