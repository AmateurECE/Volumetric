///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the application.
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

#include <argp.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib-2.0/glib.h>

#include <config.h>
#include <volumetric/configuration.h>
#include <volumetric/directory.h>
#include <volumetric/docker.h>
#include <volumetric/project-file.h>
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

static int load_volumes(VolumetricConfiguration* config) {
    ProjectIter* project_iter = project_iter_new(config);
    assert(NULL != project_iter);
    Docker* docker = docker_proxy_new();
    assert(NULL != docker);

    ProjectFile* project = NULL;
    int result = 0;
    while (NULL != (project = project_iter_next(project_iter))) {

        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, project->volumes);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            Volume* current_volume = (Volume*)value;
            // Making sure we always report an error if there's at least one.
            result += volume_checkout(current_volume, docker);
        }
    }

    docker_proxy_free(docker);
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

    result = load_volumes(&config);
    volumetric_configuration_release(&config);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
