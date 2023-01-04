///////////////////////////////////////////////////////////////////////////////
// NAME:            archive.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementations of certain routines for the archive volume
//                  type
//
// CREATED:         02/13/2022
//
// LAST EDITED:     01/04/2023
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

#ifndef VOLUMETRIC_VOLUME_ARCHIVE_H
#define VOLUMETRIC_VOLUME_ARCHIVE_H

#include <stdbool.h>

typedef struct FileContents FileContents;
typedef struct FileHash FileHash;
typedef struct Docker Docker;
typedef struct SerdecYamlDeserializer SerdecYamlDeserializer;

// An archive volume--contents are checked against a .tar.gz archive on the
// filesystem.
typedef struct ArchiveVolume {
    char* name;
    char* url;
    FileHash* hash;
    int (*update_policy)(struct ArchiveVolume*, Docker*);
    int (*commit)(struct ArchiveVolume*, Docker*);
    int (*check)(struct ArchiveVolume*, Docker*, const FileContents*);
} ArchiveVolume;

void archive_volume_defaults(ArchiveVolume* volume);
int archive_volume_deserialize_yaml(SerdecYamlDeserializer* yaml,
                                    ArchiveVolume* volume);
int archive_volume_checkout(ArchiveVolume* config, Docker* docker);
int archive_volume_diff(ArchiveVolume* volume, Docker* docker);
int archive_volume_commit(ArchiveVolume* volume, Docker* docker, bool dry_run);
void archive_volume_release(ArchiveVolume* volume);

// Update policies

typedef enum VolumetricUpdateStatus {
    VOLUMETRIC_NO_ACTION = 0,
    VOLUMETRIC_ACTION_REQUIRED = 1,
} VolumetricUpdateStatus;

int archive_volume_update_policy_never(ArchiveVolume* volume, Docker* docker);
int archive_volume_update_policy_on_stale_lock(ArchiveVolume* volume,
                                               Docker* docker);

// Commit actions

int archive_volume_commit_update_lock_file(ArchiveVolume* volume,
                                           Docker* docker);

// Check actions

int archive_volume_check_hash(ArchiveVolume* volume, Docker* docker,
                              const FileContents* file);
int archive_volume_check_remove_existing_volume(ArchiveVolume* volume,
                                                Docker* docker,
                                                const FileContents*);

#endif // VOLUMETRIC_VOLUME_ARCHIVE_H

///////////////////////////////////////////////////////////////////////////////
