///////////////////////////////////////////////////////////////////////////////
// NAME:            lock-file.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     A simple interface for interacting with archive volume lock
//                  files.
//
// CREATED:         12/30/2022
//
// LAST EDITED:	    01/01/2023
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

#ifndef VOLUMETRIC_LOCK_FILE_H
#define VOLUMETRIC_LOCK_FILE_H

#include <stddef.h>
#include <time.h>

#include <volumetric/hash.h>

typedef struct ArchiveLockFile ArchiveLockFile;

// Create a lock file for the volume with the provided name, or truncate it if
// it currently exists.
ArchiveLockFile* archive_lock_file_create(const char* volume_name);

// Attempt to open the lock file corresponding to the volume with the provided
// name, returning NULL if a lock file is not found.
ArchiveLockFile* archive_lock_file_open(const char* volume_name);

// Close the lock file.
void archive_lock_file_close(ArchiveLockFile* file);

// Get the modification time (mtime) of the open lock file.
struct timespec archive_lock_file_get_mtime(ArchiveLockFile* file);

// Get the hash stored in the lock file. The returned value is a non-owning
// pointer to the FileHash, and so should never be passed to file_hash_free().
const FileHash* archive_lock_file_get_hash(ArchiveLockFile* file);

// Update the lock file with the provided hash.
int archive_lock_file_update(ArchiveLockFile* file, const FileHash* hash);

#endif // VOLUMETRIC_LOCK_FILE_H

///////////////////////////////////////////////////////////////////////////////
