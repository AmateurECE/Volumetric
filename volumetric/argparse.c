///////////////////////////////////////////////////////////////////////////////
// NAME:            argparse.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of argument parser.
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <volumetric/argparse.h>

static const char* version = CONFIG_VERSION;
static const char* program_name = CONFIG_PROGRAM_NAME;
static const char* copyright = "\
Copyright 2021 Ethan D. Twardy. All rights reserved.\n\
This is free software; see the source for copying conditions. There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\
";

static const char* usage = "\
usage: %s [OPTIONS] <subcommand>\n\
\n\
Options:\n\
  -v:   Print program version\n\
\n\
Subcommands:\n\
  add-packaged: Add a volume to the configuration file\n\
";

static const char* add_packaged_usage = "\
usage: %s add-packaged <url>\n\
\n\
Arguments:\n\
  url:  The URL of the package to add\n\
";

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static _Noreturn void print_usage() {
    fprintf(stderr, usage, program_name);
    exit(1);
}

static _Noreturn void print_add_packaged_usage() {
    fprintf(stderr, add_packaged_usage, program_name);
    exit(1);
}

static _Noreturn void print_version() {
    printf("%s %s\n%s\n", program_name, version, copyright);
    exit(0);
}

static bool parse_flag(ArgumentParser* parser, int* index, char** argv) {
    if (!strcmp(argv[*index], "--")) {
        return false;
    }

    switch (argv[*index][1]) {
    case 'v': print_version(); break;
    default: print_usage(); break;
    }

    return true;
}

static void parse_add_packaged_subcommand_args(ArgumentParser* parser,
    int start_index, int argc, char** argv)
{
    int positional_args = 1;
    int index;
    for (index = start_index; index < argc && positional_args; ++index) {
        if (1 == positional_args) {
            parser->subcommand_args.add_packaged.url = argv[index];
            --positional_args;
        }
    }

    if (positional_args) {
        print_add_packaged_usage();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

void argparse_parse_args(ArgumentParser* parser, int argc, char** argv) {
    bool parsing_flags = true;
    int positional_args = 1;
    int index;
    for (index = 1; index < argc && positional_args; ++index) {
        if ('-' == argv[index][0] && parsing_flags) {
            parsing_flags = parse_flag(parser, &index, argv);
        }

        if (1 == positional_args) {
            parser->subcommand = argv[index];
            --positional_args;
        }
    }

    if (positional_args) {
        print_usage();
    }

    if (!strcmp("add-packaged", parser->subcommand)) {
        parse_add_packaged_subcommand_args(parser, index, argc, argv);
    } else {
        print_usage();
    }
}

///////////////////////////////////////////////////////////////////////////////
