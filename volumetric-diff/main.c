///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the utility.
//
// CREATED:         01/26/2022
//
// LAST EDITED:     02/09/2022
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
#include <errno.h>
#include <fts.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>
#include <glib-2.0/glib.h>
#include <serdec/yaml.h>

#include <config.h>
#include <volumetric/configuration.h>
#include <volumetric/docker.h>
#include <volumetric/file.h>
#include <volumetric/volume.h>
#include <volumetric-diff/directory.h>
#include <volumetric-diff/string-handling.h>

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
    case 'c': arguments->configuration_file = arg; break;
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

static int find_volume_by_name(const char* path, const char* volume_name,
    Volume* volume)
{
    DirectoryIter* iter = directory_iter_new(path);
    DirectoryEntry* entry = NULL;
    int result = 1;
    while (NULL != (entry = directory_iter_next(iter))) {
        FILE* input = fopen(entry->absolute_path, "rb");
        if (NULL == input) {
            fprintf(stderr, "error opening file %s: %s\n",
                entry->entry->d_name, strerror(errno));
        }

        VolumeFile volume_file = {0};
        SerdecYamlDeserializer* yaml = serdec_yaml_deserializer_new_file(
            input);
        result = volume_file_deserialize_from_yaml(yaml, &volume_file);
        serdec_yaml_deserializer_free(yaml);
        fclose(input);
        if (0 != result) {
            break;
        }

        GHashTableIter hash_iter;
        gpointer key, value;
        bool found = false;
        g_hash_table_iter_init(&hash_iter, volume_file.volumes);
        while (g_hash_table_iter_next(&hash_iter, &key, &value)) {
            Volume* current_volume = (Volume*)value;
            if (!strcmp(volume_name, current_volume->archive.name)) {
                g_hash_table_steal(volume_file.volumes, key);
                memcpy(volume, current_volume, sizeof(Volume));
                found = true;
                break;
            }
        }

        if (found) {
            result = 0;
            break;
        }
    }

    directory_iter_free(iter);
    return result;
}

static bool check_file_for_modifications(struct archive_entry* entry,
    const char* directory_file)
{
    // Check for differences based on stat data
    struct stat file_stat = {0};
    assert(0 == stat(directory_file, &file_stat));

    const struct stat* archive_stat = archive_entry_stat(entry);
    bool diff = false;
    if (S_ISREG(file_stat.st_mode)) {
        diff = diff || file_stat.st_size != archive_stat->st_size;
    }

    diff = diff || file_stat.st_mode != archive_stat->st_mode;
    diff = diff || file_stat.st_mtime != archive_stat->st_mtime;
    return diff;
}

// Find what's changed in the directory from the archive
static int diff_directory_from_archive(GPtrArray* directory,
    const char* archive_url, const char* directory_base)
{
    struct archive *reader = archive_read_new();
    struct archive_entry *entry = NULL;
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    FileContents archive = {0};
    file_contents_init(&archive, archive_url);
    archive_read_open_memory(reader, archive.contents, archive.size);
    while (archive_read_next_header(reader, &entry) == ARCHIVE_OK) {
        static const char* archive_base = "./";
        const char* entry_path = archive_entry_pathname(entry);
        if (!strcmp(archive_base, entry_path)) {
            // Skip "./"
            continue;
        }

        char* archive_file = string_new(entry_path + strlen(archive_base));
        size_t archive_file_length = strlen(archive_file);
        if ('/' == archive_file[archive_file_length - 1]) {
            // Truncate trailing '/'
            archive_file[archive_file_length - 1] = '\0';
        }

        bool found = false;
        for (guint i = 0; i < directory->len; ++i) {
            const char* directory_file = directory->pdata[i];
            if (!strcmp(archive_file, directory_file)) {
                found = true;
                char* full_path = string_append_new(string_new(directory_base),
                    directory_file);
                if (check_file_for_modifications(entry, full_path)) {
                    printf("M %s\n", archive_file);
                }
                g_ptr_array_remove_index_fast(directory, i);
                break;
            }
        }

        if (!found) {
            printf("D %s\n", (const char*)archive_file);
        }
        free(archive_file);
    }

    for (guint i = 0; i < directory->len && NULL != directory->pdata[i]; ++i) {
        printf("A %s\n", (const char*)directory->pdata[i]);
    }

    archive_read_free(reader);
    file_contents_release(&archive);
    return 0;
}

static GPtrArray* get_file_list_for_directory(const char* directory) {
    GPtrArray* list = g_ptr_array_new_with_free_func(free);

    char* directory_owned = string_new(directory);
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
            exit(errno);
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
            char* string = string_new(list->pdata[i] + prefix_length);
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

    GPtrArray* directory = get_file_list_for_directory(
        live_volume->mountpoint);

    // First, let's remove the top-level entry (either "./" or "/...")
    remove_matching_entry(directory, live_volume->mountpoint);
    size_t mountpoint_length = strlen(live_volume->mountpoint);
    char* mountpoint = strdup(live_volume->mountpoint);
    if ('/' != live_volume->mountpoint[mountpoint_length - 1]) {
        // Have to add that terminating '/'
        mountpoint = string_append_new(string_new(live_volume->mountpoint),
            &(char){'/'});
    }
    trim_prefix_from_entries(directory, mountpoint);

    /* for (guint i = 0; i < archive->len && NULL != archive->pdata[i]; ++i) */
    /*     { printf("%s\n", (const char*)archive->pdata[i]); } */
    FILE* output_file = fopen("directory.txt", "wb");
    for (guint i = 0; i < directory->len && NULL != directory->pdata[i]; ++i)
    { fprintf(output_file, "%s\n", (const char*)directory->pdata[i]); }
    fclose(output_file);
    result = diff_directory_from_archive(directory, volume.archive.url,
        mountpoint);
    free(mountpoint);
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
