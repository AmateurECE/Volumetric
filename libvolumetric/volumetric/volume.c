///////////////////////////////////////////////////////////////////////////////
// NAME:            volume.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for deserializing volume files.
//
// CREATED:         01/17/2022
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

#include <glib-2.0/glib.h>
#include <gobiserde/yaml.h>

#include <volumetric/hash.h>
#include <volumetric/volume.h>

const char* VOLUME_SCHEMA_VERSION = "1.0";

int archive_volume_visit_map(yaml_deserializer* yaml, void* user_data,
    const char* key)
{
    ArchiveVolume* volume = (ArchiveVolume*)user_data;
    if (!strcmp("name", key)) {
        return gobiserde_yaml_deserialize_string(yaml, &volume->name);
    } else if (!strcmp("url", key)) {
        return gobiserde_yaml_deserialize_string(yaml, &volume->url);
    } else {
        FileHashType hash_type = string_to_file_hash_type(key);
        if (FILE_HASH_TYPE_INVALID == hash_type) {
            return -EINVAL;
        }
        volume->hash = malloc(sizeof(FileHash));
        assert(NULL != volume->hash);
        volume->hash->type = hash_type;
        return gobiserde_yaml_deserialize_string(yaml,
            &volume->hash->hash_string);
    }
}

int volume_type_visit_map(yaml_deserializer* yaml, void* user_data,
    const char* key)
{
    Volume* volume = (Volume*)user_data;
    if (!strcmp("archive", key)) {
        volume->type = VOLUME_TYPE_ARCHIVE;
        return gobiserde_yaml_deserialize_map(yaml, archive_volume_visit_map,
            &volume->archive);
    } else {
        return -EINVAL;
    }
}

int volume_visit_map(yaml_deserializer* yaml, void* user_data, const char* key)
{
    GHashTable* volumes = (GHashTable*)user_data;
    Volume* volume = malloc(sizeof(Volume));
    if (NULL == volume) {
        return -ENOMEM;
    }

    gchar* owned_name = (gchar*)strdup(key);
    int result = gobiserde_yaml_deserialize_map(yaml, volume_type_visit_map,
        volume);
    g_hash_table_insert(volumes, owned_name, volume);
    return result;
}

int volume_file_visit_map(yaml_deserializer* yaml, void* user_data,
    const char* key)
{
    VolumeFile* volumes = (VolumeFile*)user_data;
    if (!strcmp("version", key)) {
        int result = gobiserde_yaml_deserialize_string(yaml,
            &volumes->version);
        if (0 >= result) {
            return result;
        } else if (strcmp(VOLUME_SCHEMA_VERSION, volumes->version)) {
            return -EINVAL;
        }
        return 1;
    } else if (!strcmp("volumes", key)) {
        return gobiserde_yaml_deserialize_map(yaml, volume_visit_map,
            volumes->volumes);
    } else {
        return -ENOSYS;
    }
}

static void volume_free(void* volume) {
    // TODO
}

static void volume_file_defaults(VolumeFile* volumes) {
    volumes->volumes = g_hash_table_new_full(g_str_hash, g_str_equal, free,
        volume_free);
}

// Deserialize the VolumeFile instance from the deserializer
int volume_file_deserialize_from_yaml(yaml_deserializer* yaml,
    VolumeFile* volumes)
{
    volume_file_defaults(volumes);
    int result = gobiserde_yaml_deserialize_map(yaml, volume_file_visit_map,
        volumes);
    if (0 > result) {
        return -EINVAL;
    }

    return 0;
}

// Free memory used internally by the instance
void volume_file_release(VolumeFile* volumes) {
    free(volumes->version);
    g_hash_table_unref(volumes->volumes);
}

///////////////////////////////////////////////////////////////////////////////
