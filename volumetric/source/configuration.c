///////////////////////////////////////////////////////////////////////////////
// NAME:            configuration.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for loading configuration file.
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

#include <stdio.h>
#include <string.h>

#include <gobiserde/yaml.h>

#include <configuration.h>

const char* CONFIGURATION_CURRENT_VERSION = "1.0";

static int visit_mapping(yaml_deserializer* deser, void* user_data,
    const char* key)
{
    VolumetricConfiguration* config = (VolumetricConfiguration*)user_data;
    int result = 0;
    if (!strcmp("version", key)) {
        // Version check. The version affects whether we can fully deserialize
        // the configuration, so we do need to check it here.
        result = gobiserde_yaml_deserialize_string(deser,
            &config->version);
        if (0 >= result) {
            return result;
        } else if (strcmp(CONFIGURATION_CURRENT_VERSION, config->version)) {
            return -PARSE_VERSION_MISMATCH;
        }
    }

    else if (!strcmp("volume-directory", key)) {
        result = gobiserde_yaml_deserialize_string(deser,
            &config->volume_directory);
    }

    return result;
}

ParseResult volumetric_configuration_deserialize_yaml(yaml_deserializer* deser,
    VolumetricConfiguration* config)
{
    return gobiserde_yaml_deserialize_map(deser, visit_mapping, config);
}

///////////////////////////////////////////////////////////////////////////////
