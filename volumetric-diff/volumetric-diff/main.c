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
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <glib-2.0/glib.h>
#include <gobiserde/yaml.h>

#include <config.h>
#include <volumetric/configuration.h>
#include <volumetric/volume.h>

const char* argp_program_version = "volumetric-diff " CONFIG_VERSION;
const char* argp_program_bug_address = "<ethan.twardy@gmail.com>";
static char doc[] = "Check for modifications in live configuration";
static char args_doc[] = "VOLUME_NAME";
static const int NUMBER_OF_ARGS = 1;
static struct argp_option options[] = {
    {"config", 'c', "FILE", 0,
     "Read configuration file FILE instead of default ("
     CONFIG_CONFIGURATION_FILE ")", 0},
    { 0 },
};

static const char* CONFIGURATION_FILE = CONFIG_CONFIGURATION_FILE;

struct arguments {
    const char* volume_name;
    const char* configuration_file;
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = state->input;
    switch (key) {
    case 'c':
        arguments->configuration_file = arg;
        break;

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

struct VolumeFilter {
    const char* name;
    bool found;
};

static void compare_volume_name(void* key __attribute__((unused)), void* value,
    void* user_data)
{
    struct VolumeFilter* filter = (struct VolumeFilter*)user_data;
    Volume* volume = (Volume*)value;
    if (!strcmp(filter->name, volume->archive.name)) {
        filter->found = true;
    }
}

static int find_volume_by_name(const char* path, const char* volume_name,
    Volume* volume)
{
    struct dirent* entry = NULL;
    DIR* directory = opendir(path);
    if (NULL == directory) {
        fprintf(stderr, "cannot open directory %s: %s\n", path,
            strerror(errno));
        return errno;
    }

    int result = 0;
    size_t path_length = strlen(path);
    char* whole_path = malloc(path_length + 256);
    if (NULL == whole_path) {
        fprintf(stderr, "Cannot allocate memory: %s\n", strerror(errno));
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

        FILE* input = fopen(whole_path, "rb");
        if (NULL == input) {
            fprintf(stderr, "error opening file %s: %s\n", entry->d_name,
                strerror(errno));
        }

        VolumeFile volume_file = {0};
        yaml_deserializer* yaml = gobiserde_yaml_deserializer_new_file(input);
        result = volume_file_deserialize_from_yaml(yaml, &volume_file);
        if (0 != result) {
            break;
        }

        struct VolumeFilter filter = {0};
        filter.name = volume_name;
        g_hash_table_foreach(volume_file.volumes, compare_volume_name,
            &filter);

        gobiserde_yaml_deserializer_free(&yaml);
        fclose(input);
    }

    free(whole_path);
    closedir(directory);
    return result;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };
int main(int argc, char** argv) {
    struct arguments arguments = {0};
    arguments.configuration_file = CONFIGURATION_FILE;
    int result = 0;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    VolumetricConfiguration config = {0};
    result = volumetric_configuration_load(arguments.configuration_file,
        &config);
    assert(result);

    Volume volume = {0};
    result = find_volume_by_name(config.volume_directory,
        arguments.volume_name, &volume);
    assert(result);

    printf("volume.url=%s\n", volume.archive.url);

    volumetric_configuration_release(&config);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
