///////////////////////////////////////////////////////////////////////////////
// NAME:            volume.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to load and version volumes from configuration files.
//
// CREATED:         01/17/2022
//
// LAST EDITED:     01/17/2022
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

typedef struct _GHashTable GHashTable;
typedef struct yaml_deserializer yaml_deserializer;

// A volume file currently looks like this:
// version: '1.0'
//
// volume-path: <colon-separated list of EXTRA paths to search for volumes>
//
// volumes:
//  <name>:
//   <type, e.g. archive>:
//    name: <name of the volume>
//    url: <url to find the volume at. Only file:// scheme is supported>
//    hash: <hash of the volume file>

typedef struct ArchiveVolume {
    char* name;
    char* url;
    char* hash;
} ArchiveVolume;

typedef enum VolumeType {
    VOLUME_TYPE_ARCHIVE,
} VolumeType;

typedef struct Volume {
    VolumeType type;
    union {
        ArchiveVolume archive;
    };
} Volume;

typedef struct VolumeFile {
    // Mostly used internally--to check that the schema of this file is what
    // we expect.
    char* version;

    // Hash table of volumes.
    GHashTable* volumes;
} VolumeFile;

extern const char* VOLUME_SCHEMA_VERSION; // 1.0

// Deserialize the VolumeFile instance from the deserializer
int volume_file_deserialize_from_yaml(yaml_deserializer* yaml,
    VolumeFile* volumes);

// Free memory used internally by the instance
void volume_file_release(VolumeFile* volumes);

#endif // VOLUMETRIC_VOLUME_H

///////////////////////////////////////////////////////////////////////////////
