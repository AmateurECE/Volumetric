///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the utility.
//
// CREATED:         01/26/2022
//
// LAST EDITED:     01/28/2022
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
#include <fts.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <archive.h>
#include <archive_entry.h>
#include <glib-2.0/glib.h>
#include <gobiserde/yaml.h>

#include <config.h>
#include <volumetric/configuration.h>
#include <volumetric/docker.h>
#include <volumetric/file.h>
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
    Volume* volume;
    void* key;
    bool found;
};

static void compare_volume_name(void* key, void* value, void* user_data) {
    struct VolumeFilter* filter = (struct VolumeFilter*)user_data;
    Volume* volume = (Volume*)value;
    if (!strcmp(filter->name, volume->archive.name)) {
        filter->found = true;
        filter->key = key;
        filter->volume = volume;
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

    int result = 1;
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
        gobiserde_yaml_deserializer_free(&yaml);
        fclose(input);
        if (0 != result) {
            break;
        }

        struct VolumeFilter filter = {0};
        filter.name = volume_name;
        g_hash_table_foreach(volume_file.volumes, compare_volume_name,
            &filter);
        if (filter.found) {
            g_hash_table_steal(volume_file.volumes, filter.key);
            memcpy(volume, filter.volume, sizeof(Volume));
            result = 0;
            break;
        }
    }

    free(whole_path);
    closedir(directory);
    return result;
}

static int diff_file_lists(GPtrArray* left, GPtrArray* right) {
    for (guint i = 0; i < left->len && NULL != left->pdata[i]; ++i) {
        printf("left: %s\n", (const char*)left->pdata[i]);
    }

    for (guint i = 0; i < right->len && NULL != right->pdata[i]; ++i) {
        printf("right: %s\n", (const char*)right->pdata[i]);
    }
    return 0;
}

static GPtrArray* get_file_list_for_archive(const char* archive_path) {
    GPtrArray* list = g_ptr_array_new_with_free_func(free);
    struct archive *reader = archive_read_new();
    struct archive_entry *entry = NULL;
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    FileContents archive = {0};
    file_contents_init(&archive, archive_path);
    archive_read_open_memory(reader, archive.contents, archive.size);
    while (archive_read_next_header(reader, &entry) == ARCHIVE_OK) {
        g_ptr_array_add(list, (gpointer)strdup(archive_entry_pathname(entry)));
    }

    archive_read_free(reader);
    file_contents_release(&archive);
    return list;
}

static GPtrArray* get_file_list_for_directory(const char* directory) {
    GPtrArray* list = g_ptr_array_new_with_free_func(free);

    char* directory_owned = malloc(strlen(directory) + 1);
    assert(NULL != directory_owned);
    strcpy(directory_owned, directory);
    char* const paths[] = {directory_owned, NULL};
    FTS* tree = fts_open(paths, FTS_NOCHDIR, 0);
    assert(NULL != tree);

    // TODO: How does this work with symlinks? I'd expect they would break.
    FTSENT* node = NULL;
    while ((node = fts_read(tree))) {
        if (FTS_F == node->fts_info || FTS_D == node->fts_info) {
            g_ptr_array_add(list, strdup(node->fts_path));
        } else if (FTS_ERR == node->fts_info || FTS_DNR == node->fts_info
            || FTS_NS == node->fts_info) {
            fprintf(stderr, "fts_read error: %s\n", strerror(node->fts_errno));
        }
    }

    fts_close(tree);
    free(directory_owned);
    return list;
}

static void remove_matching_entry(GPtrArray* list, const char* match) {
    // First, remove the entry corresponding to the match
    guint index = 0;
    for (guint i = 0; i < list->len && NULL != list->pdata[i]; ++i) {
        if (!strcmp(match, list->pdata[i])) {
            index = i;
            break;
        }
    }
    g_ptr_array_remove_index_fast(list, index);
}

static void trim_prefix_from_entries(GPtrArray* list, const char* prefix) {
    // Then, remove the prefix from any entries that have it
    size_t prefix_length = strlen(prefix);
    for (guint i = 0; i < list->len && NULL != list->pdata[i]; ++i) {
        if (!strncmp(prefix, list->pdata[i], prefix_length)) {
            size_t string_length = strlen(list->pdata[i]);
            char* string = malloc(string_length - prefix_length + 1);
            assert(NULL != string);
            strcpy(string, list->pdata[i] + prefix_length);
            string[string_length] = '\0';
            free(list->pdata[i]);
            list->pdata[i] = string;
        }
    }
}

static int diff_volume(const char* volume_directory, const char* volume_name) {
    Volume volume = {0};
    int result = find_volume_by_name(volume_directory, volume_name, &volume);
    assert(0 == result);

    Docker* docker = docker_proxy_new();
    DockerVolume* live_volume = docker_volume_inspect(docker,
        volume.archive.name);
    docker_proxy_free(docker);
    assert(NULL != live_volume);

    GPtrArray* archive = get_file_list_for_archive(volume.archive.url);
    GPtrArray* directory = get_file_list_for_directory(
        live_volume->mountpoint);

    // First, let's remove the top-level entry (either "./" or "/...")
    remove_matching_entry(archive, "./");
    trim_prefix_from_entries(archive, "./");

    remove_matching_entry(directory, live_volume->mountpoint);
    size_t mountpoint_length = strlen(live_volume->mountpoint);
    if ('/' != live_volume->mountpoint[mountpoint_length - 1]) {
        // Have to add that terminating '/'
        char* mountpoint = malloc(mountpoint_length + 2);
        strcpy(mountpoint, live_volume->mountpoint);
        mountpoint[mountpoint_length] = '/';
        mountpoint[mountpoint_length + 1] = '\0';
        trim_prefix_from_entries(directory, mountpoint);
        free(mountpoint);
    } else {
        trim_prefix_from_entries(directory, live_volume->mountpoint);
    }

    result = diff_file_lists(archive, directory);
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
    assert(0 == result);

    // Do diff using volume
    result = diff_volume(config.volume_directory, arguments.volume_name);

    volumetric_configuration_release(&config);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
