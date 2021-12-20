///////////////////////////////////////////////////////////////////////////////
// NAME:            configuration.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for handling configuration files.
//
// CREATED:         12/19/2021
//
// LAST EDITED:     12/19/2021
//
// Copyright 2021, Ethan D. Twardy
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

typedef struct _GArray GArray;

enum {
    MAJOR_VERSION_SHIFT=24,
    MINOR_VERSION_MASK=8,
};

enum {
    MAJOR_VERSION_WIDTH=8,
    MINOR_VERSION_WIDTH=16,
};

typedef struct VolumetricConfig {
    int version;
    GArray* volumes;
} VolumetricConfig;

VolumetricConfig* load_configuration(const char* configuration_file);
int persist_configuration(VolumetricConfig* config,
    const char* configuration_file);

#endif // VOLUMETRIC_CONFIGURATION_H

///////////////////////////////////////////////////////////////////////////////
