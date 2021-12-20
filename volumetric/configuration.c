///////////////////////////////////////////////////////////////////////////////
// NAME:            configuration.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for handling configuration.
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <volumetric/configuration.h>

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static void load_configuration_V1_0(VolumetricConfig* config,
    yaml_parser_t* parser)
{}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

VolumetricConfig* load_configuration(const char* configuration_file) {
    VolumetricConfig* config = malloc(sizeof(VolumetricConfig));
    if (NULL == config) {
        fprintf(stderr, "Couldn't load input file: %s\n", strerror(errno));
        exit(1);
    }

    FILE* input_file = fopen(configuration_file, "rb");
    if (NULL == input_file) {
        fprintf(stderr, "Couldn't load input file: %s\n", strerror(errno));
        exit(1);
    }

    yaml_parser_t parser;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_string(&parser, input_file);

    bool done = false;
    yaml_event_t event;
    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            break;
        }

        done == (YAML_STREAM_END_EVENT == event.type);
        yaml_event_delete(&event);
    }
}

int persist_configuration(VolumetricConfig* config,
    const char* configuration_file)
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
