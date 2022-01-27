///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the utility.
//
// CREATED:         01/26/2022
//
// LAST EDITED:     01/26/2022
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

#include <argp.h>
#include <config.h>

const char* argp_program_version = "volumetric-diff " CONFIG_VERSION;
const char* argp_program_bug_address = "<ethan.twardy@gmail.com>";
static char doc[] = "Check for modifications in live configuration";
static char args_doc[] = "VOLUME_NAME";
static const int NUMBER_OF_ARGS = 1;
static struct argp_option options[] = {{ 0 },};
struct arguments { const char* volume_name; };

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = state->input;
    switch (key) {
    case ARGP_KEY_ARG:
        if (state->arg_num >= NUMBER_OF_ARGS) {
            argp_usage(state);
        }

        arguments->volume_name = arg;
        break;

    case ARGP_KEY_END:
        if (state->arg_num < NUMBER_OF_ARGS) {
            argp_usage(state);
        }

        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };
int main(int argc, char** argv) {
    struct arguments arguments = {0};
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
}

///////////////////////////////////////////////////////////////////////////////
