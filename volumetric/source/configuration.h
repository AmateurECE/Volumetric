///////////////////////////////////////////////////////////////////////////////
// NAME:            configuration.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for deserializing configuration data from YAML.
//
// CREATED:         01/16/2022
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

#ifndef VOLUMETRIC_CONFIGURATION_H
#define VOLUMETRIC_CONFIGURATION_H

typedef struct yaml_deserializer yaml_deserializer;

typedef struct VolumetricConfiguration {
    char* version;
    char* volume_directory;
} VolumetricConfiguration;

int volumetric_configuration_deserialize_yaml(yaml_deserializer* deser,
    VolumetricConfiguration* config);

#endif // VOLUMETRIC_CONFIGURATION_H

///////////////////////////////////////////////////////////////////////////////
