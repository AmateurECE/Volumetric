///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the application.
//
// CREATED:         01/16/2022
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
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <volumetric/configuration.h>
#include <volumetric/versioning.h>
#include <volumetric/volume.h>

const char* argp_program_version = "volumetric " CONFIG_VERSION;
const char* argp_program_bug_address = "<ethan.twardy@gmail.com>";
static char doc[] = "Version control for Docker persistent volumes";
static struct argp_option options[] = {
    {"config", 'c', "FILE", 0,
     "Read configuration file FILE instead of default ("
     CONFIG_CONFIGURATION_FILE ")", 0},
    { 0 },
};
static char args_doc[] = "";

struct arguments {
    const char* configuration_file;
};

static const char* CONFIGURATION_FILE = CONFIG_CONFIGURATION_FILE;

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = (struct arguments*)state->input;
    switch (key) {
    case 'c':
        arguments->configuration_file = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static void print_error(const char* message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    fprintf(stderr, ": %s\n", strerror(errno));
}

static int load_volumes(const char* path, int (*load_entry)(FILE*)) {
    struct dirent* entry = NULL;
    DIR* directory = opendir(path);
    if (NULL == directory) {
        print_error("cannot open directory %s", path);
        return errno;
    }

    int result = 0;
    size_t path_length = strlen(path);
    char* whole_path = malloc(path_length + 256);
    if (NULL == whole_path) {
        print_error("Cannot allocate memory");
    }

    memset(whole_path, 0, path_length + 256);
    strcat(whole_path, path);
    whole_path[path_length] = '/';

    while (NULL != (entry = readdir(directory))) {
        // Ignore '.' and '..'
        if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name)) {
            continue;
        }

        size_t entry_length = strlen(entry->d_name);
        memcpy(whole_path + path_length + 1, entry->d_name, entry_length);
        whole_path[path_length + 1 + entry_length] = '\0';

        FILE* input_file = fopen(whole_path, "rb");
        if (NULL == input_file) {
            print_error("error opening file %s", entry->d_name);
        }

        result = load_entry(input_file);
        fclose(input_file);
        if (0 != result) {
            break;
        }
    }

    free(whole_path);
    closedir(directory);
    return result;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char** argv) {
    struct arguments arguments = {
        .configuration_file = CONFIGURATION_FILE,
    };

    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    VolumetricConfiguration config = {0};
    int result = volumetric_configuration_load(arguments.configuration_file,
        &config);
    if (0 != result) {
        return result;
    }

    result = load_volumes(config.volume_directory, version_volumes_in_file);
    volumetric_configuration_release(&config);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
