///////////////////////////////////////////////////////////////////////////////
// NAME:            volume.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to load and version volumes from configuration files.
//
// CREATED:         01/17/2022
//
// LAST EDITED:     02/14/2022
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

#ifndef VOLUMETRIC_VOLUME_H
#define VOLUMETRIC_VOLUME_H

#include <stdbool.h>

#include <volumetric/volume/archive.h>

typedef struct Docker Docker;
typedef struct SerdecYamlDeserializer SerdecYamlDeserializer;

// TODO: Make these types opaque?
// Volume-type-generic container
typedef enum VolumeType {
    VOLUME_TYPE_ARCHIVE,
} VolumeType;

typedef struct Volume {
    VolumeType type;
    union {
        ArchiveVolume archive;
    };
} Volume;

// De-serialize a Volume instance from a Yaml Deserializer
int volume_deserialize_yaml(SerdecYamlDeserializer* yaml, Volume* volume);

// Free memory consumed by a volume
void volume_free(Volume* volume);    // Free <volume>
void volume_release(Volume* volume); // Don't free <volume>

// "Version" the volume from its source
int volume_checkout(Volume* volume, Docker* docker);

// Check for differences between the volume source and live
int volume_diff(Volume* volume, Docker* docker);

// Commit a dirty volume to the source
int volume_commit(Volume* volume, Docker* docker, bool dry_run);

#endif // VOLUMETRIC_VOLUME_H

///////////////////////////////////////////////////////////////////////////////
