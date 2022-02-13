///////////////////////////////////////////////////////////////////////////////
// NAME:            configuration.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for deserializing configuration data from YAML.
//
// CREATED:         01/16/2022
//
// LAST EDITED:     02/13/2022
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

#ifndef VOLUMETRIC_CONFIGURATION_H
#define VOLUMETRIC_CONFIGURATION_H

#include <stdbool.h>

// Keys currently supported in configuration:
// version:
//  type: string
//  required: true
//  value: 1.0
//  description: Current version of configuration schema
//
// volume-directory:
//  type: string
//  required: true
//  description: Location to search for volume configurations
//
// volume-path:
//  type: string
//  description: Colon-separated list of paths to search for volume images

typedef struct VolumetricConfiguration {
    char* version;
    char* volume_directory;
    char* volume_path;
} VolumetricConfiguration;

typedef enum ParseResult {
    PARSE_OK,
    PARSE_VERSION_MISMATCH,
} ParseResult;

extern const char* CONFIGURATION_CURRENT_VERSION;

///////////////////////////////////////////////////////////////////////////////
// Configuration loading API (lower-level)
////

ParseResult volumetric_configuration_load(const char* file_path,
    VolumetricConfiguration* configuration);
void volumetric_configuration_release(VolumetricConfiguration* config);

///////////////////////////////////////////////////////////////////////////////
// Iteration API
////

typedef struct ProjectIter ProjectIter;
typedef struct ProjectFile ProjectFile;
ProjectIter* project_iter_new(VolumetricConfiguration* configuration);
ProjectFile* project_iter_next(ProjectIter* iter);
void project_iter_free(ProjectIter* iter);

///////////////////////////////////////////////////////////////////////////////
// Search API
////

typedef struct Volume Volume;
bool volumetric_configuration_find_volume_by_name(
    VolumetricConfiguration* config, const char* volume_name, Volume* volume);

#endif // VOLUMETRIC_CONFIGURATION_H

///////////////////////////////////////////////////////////////////////////////
