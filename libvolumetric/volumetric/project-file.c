///////////////////////////////////////////////////////////////////////////////
// NAME:            project-file.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for serializing/deserializing project files.
//
// CREATED:         02/09/2022
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

#include <errno.h>

#include <glib-2.0/glib.h>
#include <serdec/yaml.h>

#include <volumetric/project-file.h>
#include <volumetric/volume.h>

const char* VOLUME_SCHEMA_VERSION = "1.0";

static int volume_visit_map(SerdecYamlDeserializer* yaml, void* user_data,
    const char* key)
{
    GHashTable* volumes = (GHashTable*)user_data;
    Volume* volume = malloc(sizeof(Volume));
    if (NULL == volume) {
        return -ENOMEM;
    }

    gchar* owned_name = (gchar*)strdup(key);
    int result = volume_deserialize_yaml(yaml, volume);
    g_hash_table_insert(volumes, owned_name, volume);
    return result;
}

static int project_file_visit_map(SerdecYamlDeserializer* yaml,
    void* user_data, const char* key)
{
    ProjectFile* volumes = (ProjectFile*)user_data;
    if (!strcmp("version", key)) {
        const char* temp = NULL;
        int result = serdec_yaml_deserialize_string(yaml, &temp);
        volumes->version = strdup(temp);
        if (0 >= result) {
            return result;
        } else if (strcmp(VOLUME_SCHEMA_VERSION, volumes->version)) {
            return -EINVAL;
        }
        return 1;
    } else if (!strcmp("volumes", key)) {
        return serdec_yaml_deserialize_map(yaml, volume_visit_map,
            volumes->volumes);
    } else {
        return -ENOSYS;
    }
}

static void project_file_defaults(ProjectFile* volumes) {
    volumes->volumes = g_hash_table_new_full(g_str_hash, g_str_equal, free,
        (GDestroyNotify)volume_free);
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

// Deserialize the ProjectFile instance from the deserializer
int project_file_deserialize_from_yaml(SerdecYamlDeserializer* yaml,
    ProjectFile* volumes)
{
    project_file_defaults(volumes);
    int result = serdec_yaml_deserialize_map(yaml, project_file_visit_map,
        volumes);
    if (0 > result) {
        return -EINVAL;
    }

    return 0;
}

// Free memory used internally by the instance
void project_file_release(ProjectFile* volumes) {
    free(volumes->version);
    g_hash_table_unref(volumes->volumes);
}

///////////////////////////////////////////////////////////////////////////////
