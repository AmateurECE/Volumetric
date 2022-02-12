///////////////////////////////////////////////////////////////////////////////
// NAME:            configuration.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for loading configuration file.
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

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib-2.0/glib.h>
#include <serdec/yaml.h>

#include <volumetric/configuration.h>
#include <volumetric/directory.h>
#include <volumetric/project-file.h>
#include <volumetric/volume.h>

const char* CONFIGURATION_CURRENT_VERSION = "1.0";

typedef struct ProjectIter {
    DirectoryIter* iter;
    DirectoryEntry* entry;
    ProjectFile project;
} ProjectIter;

///////////////////////////////////////////////////////////////////////////////
// Private Functions
////

static void volumetric_configuration_defaults(VolumetricConfiguration* config)
{ config->volume_path = strdup(""); }

static int visit_mapping(SerdecYamlDeserializer* deser, void* user_data,
    const char* key)
{
    VolumetricConfiguration* config = (VolumetricConfiguration*)user_data;
    int result = 0;
    const char* temp = NULL;
    if (!strcmp("version", key)) {
        // Version check. The version affects whether we can fully deserialize
        // the configuration, so we do need to check it here.
        result = serdec_yaml_deserialize_string(deser, &temp);
        config->version = strdup(temp);
        if (0 >= result) {
            return result;
        } else if (strcmp(CONFIGURATION_CURRENT_VERSION, config->version)) {
            return -PARSE_VERSION_MISMATCH;
        }
    }

    else if (!strcmp("volume-directory", key)) {
        result = serdec_yaml_deserialize_string(deser, &temp);
        config->volume_directory = strdup(temp);
    }

    else if (!strcmp("volume-path", key)) {
        result = serdec_yaml_deserialize_string(deser, &temp);
        free(config->volume_path);
        config->volume_path = strdup(temp);
    }

    return result;
}

static ParseResult volumetric_configuration_deserialize_yaml(
    SerdecYamlDeserializer* deser, VolumetricConfiguration* config)
{
    volumetric_configuration_defaults(config);
    int result = serdec_yaml_deserialize_map(deser, visit_mapping, config);
    if (0 > result) {
        return -EINVAL;
    }

    return 0;
}

static char* get_volume_directory(const char* configuration_file,
    const char* volume_directory)
{
    // See dirname(3). This string is not free'd.
    char* owned_configuration_file = strdup(configuration_file);
    char* conf_directory = dirname(owned_configuration_file);
    size_t path_size = strlen(conf_directory) + 1 + strlen(volume_directory);
    char* path = malloc(path_size + 1);
    if (NULL == path) {
        return NULL;
    }

    memset(path, 0, path_size + 1);
    strcat(path, conf_directory);
    path[strlen(conf_directory)] = '/';
    strcat(path, volume_directory);
    path[path_size] = '\0';

    // Memory cleanup
    free(owned_configuration_file);
    return path;
}

///////////////////////////////////////////////////////////////////////////////
// Public Configuration Deserialization API
////

ParseResult volumetric_configuration_load(const char* config_file,
    VolumetricConfiguration* config)
{
    FILE* input_file = fopen(config_file, "rb");
    if (NULL == input_file) {
        fprintf(stderr, "Couldn't open configuration file %s: %s\n",
            config_file, strerror(errno));
        return errno;
    }

    SerdecYamlDeserializer* deser = serdec_yaml_deserializer_new_file(
        input_file);
    int result = volumetric_configuration_deserialize_yaml(deser, config);
    serdec_yaml_deserializer_free(deser);
    fclose(input_file);

    if (-PARSE_VERSION_MISMATCH == result) {
        fprintf(stderr, "Configuration file %s: version mismatch, expected %s",
            config_file, CONFIGURATION_CURRENT_VERSION);
    }

    // Take the path of volume_directory relative to the dirname of
    // conf_directory, if volume_directory is not an absolute path.
    if ('/' != config->volume_directory[0]) {
        char* path = get_volume_directory(config_file,
            config->volume_directory);
        free(config->volume_directory);
        config->volume_directory = path;
    }

    return result;
}

void volumetric_configuration_release(VolumetricConfiguration* config) {
    if (NULL != config->version) { free(config->version); }
    if (NULL != config->volume_directory) { free(config->volume_directory); }
    if (NULL != config->volume_path) { free(config->volume_path); }
}

///////////////////////////////////////////////////////////////////////////////
// Public Volume Iteration API
////

ProjectIter* project_iter_new(VolumetricConfiguration* configuration) {
    ProjectIter* iter = malloc(sizeof(ProjectIter));
    if (NULL == iter) {
        return NULL;
    }

    memset(iter, 0, sizeof(*iter));
    iter->iter = directory_iter_new(configuration->volume_directory);
    iter->entry = NULL;
    return iter;
}

ProjectFile* project_iter_next(ProjectIter* iter) {
    iter->entry = directory_iter_next(iter->iter);
    if (NULL == iter->entry) {
        return NULL;
    }

    FILE* input = fopen(iter->entry->absolute_path, "rb");
    if (NULL == input) {
        fprintf(stderr, "error opening file %s: %s\n",
            iter->entry->entry->d_name, strerror(errno));
        return NULL;
    }

    SerdecYamlDeserializer* yaml = serdec_yaml_deserializer_new_file(input);
    int result = project_file_deserialize_from_yaml(yaml, &iter->project);
    serdec_yaml_deserializer_free(yaml);
    fclose(input);
    if (0 != result) {
        return NULL;
    }

    return &iter->project;
}

void project_iter_free(ProjectIter* iter) {
    directory_iter_free(iter->iter);
    free(iter);
}

///////////////////////////////////////////////////////////////////////////////
