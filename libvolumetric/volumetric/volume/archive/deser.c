///////////////////////////////////////////////////////////////////////////////
// NAME:            deser.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Routines for serializing and deserializing archive volumes
//
// CREATED:         02/13/2022
//
// LAST EDITED:     06/22/2022
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
#include <stdlib.h>
#include <string.h>

#include <serdec/yaml.h>

#include <volumetric/hash.h>
#include <volumetric/volume/archive.h>

static int archive_volume_visit_map(SerdecYamlDeserializer* yaml,
    void* user_data, const char* key)
{
    ArchiveVolume* volume = (ArchiveVolume*)user_data;
    const char* temp = NULL;

    if (!strcmp("name", key)) {
        int result = serdec_yaml_deserialize_string(yaml, &temp);
        volume->name = strdup(temp);
        return result;
    } else if (!strcmp("url", key)) {
        int result = serdec_yaml_deserialize_string(yaml, &temp);
        volume->url = strdup(temp);
        return result;
    } else {
        int result = serdec_yaml_deserialize_string(yaml, &temp);
        volume->hash = file_hash_from_string(key, temp);
        if (NULL == volume->hash) {
            return -EINVAL;
        }
        return result;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

int archive_volume_deserialize_yaml(SerdecYamlDeserializer* yaml,
    ArchiveVolume* volume)
{ return serdec_yaml_deserialize_map(yaml, archive_volume_visit_map, volume); }

void archive_volume_release(ArchiveVolume* volume) {
    free(volume->name);
    free(volume->url);
    file_hash_free(volume->hash);
}

///////////////////////////////////////////////////////////////////////////////
