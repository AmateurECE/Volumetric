///////////////////////////////////////////////////////////////////////////////
// NAME:            volume.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to load and version volumes from configuration files.
//
// CREATED:         01/17/2022
//
// LAST EDITED:     02/11/2022
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

typedef struct FileHash FileHash;
typedef struct SerdecYamlDeserializer SerdecYamlDeserializer;

// An archive volume--contents are checked against a .tar.gz archive on the
// filesystem.
typedef struct ArchiveVolume {
    char* name;
    char* url;
    FileHash* hash;
} ArchiveVolume;

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
void volume_free(Volume* volume);

#endif // VOLUMETRIC_VOLUME_H

///////////////////////////////////////////////////////////////////////////////
