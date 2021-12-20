///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the application.
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

#include <stdio.h>
#include <string.h>

#include <volumetric/argparse.h>
#include <volumetric/configuration.h>
#include <volumetric/subcommand.h>

static const char* CONFIGURATION_FILE = "volumetric.yaml";

int main(int argc, char** argv) {
    ArgumentParser parser = {0};
    argparse_parse_args(&parser, argc, argv);

    if (!strcmp("add-packaged", parser.subcommand)) {
        VolumetricConfig* config = load_configuration(
            CONFIGURATION_FILE);
        volumetric_add_packaged(config,
            parser.subcommand_args.add_packaged.url);
        int result = persist_configuration(config, CONFIGURATION_FILE);
        if (result) {
            fprintf(stderr, "Couldn't write configuration to %s: %s\n",
                CONFIGURATION_FILE, strerror(result));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
