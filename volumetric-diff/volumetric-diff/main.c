///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the utility.
//
// CREATED:         01/26/2022
//
// LAST EDITED:     02/02/2022
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

#include <archive.h>
#include <archive_entry.h>
#include <glib-2.0/glib.h>
#include <gobiserde/yaml.h>

#include <config.h>
#include <volumetric/configuration.h>
#include <volumetric/docker.h>
#include <volumetric/file.h>
#include <volumetric/volume.h>
#include <volumetric-diff/directory.h>
#include <volumetric-diff/string.h>

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
        yaml_deserializer* yaml = gobiserde_yaml_deserializer_new_file(input);
        result = volume_file_deserialize_from_yaml(yaml, &volume_file);
        gobiserde_yaml_deserializer_free(&yaml);
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

static bool check_file_for_modifications(const char* left,
    const char* archive_url,  const char* archive_base,
    const char* directory_base)
{
    struct archive *reader = archive_read_new();
    struct archive_entry *entry = NULL;
    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    FileContents archive_file = {0};
    file_contents_init(&archive_file, archive_url);
    archive_read_open_memory(reader, archive_file.contents, archive_file.size);

    // Iterate through the archive to find the entry for this file.
    char* archive_path = string_append_new(string_new(archive_base), left);
    assert(NULL != archive_path);
    while (archive_read_next_header(reader, &entry) == ARCHIVE_OK
        && strcmp(archive_entry_pathname(entry), archive_path));
    free(archive_path);

    // Open up the actual file on disk
    FileContents live_file = {0};
    char* path_url = string_join_new(string_new(directory_base), '/', left);
    file_contents_init(&live_file, path_url);
    free(path_url);

    size_t live_file_index = 0;
    bool different = false;
    la_ssize_t bytes_read = 0;
    char buffer[4096];
    while (!different && 0 < (bytes_read = archive_read_data(reader, buffer,
                sizeof(buffer))))
    {
        if (live_file_index + bytes_read > live_file.size) {
            different = true;
        } else {
            different = !!strncmp(buffer, live_file.contents + live_file_index,
                bytes_read);
            live_file_index += bytes_read;
        }
    }

    if (live_file_index < live_file.size) {
        different = true;
    }

    archive_read_free(reader);
    file_contents_release(&archive_file);
    file_contents_release(&live_file);
    return different;
}

static bool check_list_for_file(const char* file, gpointer* pdata,
    guint len)
{
    for (guint i = 0; i < len; ++i) {
        if (!strcmp(file, (const char*)pdata[i])) {
            return true;
        }
    }

    return false;
}

// Find what's changed in the directory from the archive
static int diff_directory_from_archive(GPtrArray* archive,
    GPtrArray* directory, const char* archive_url, const char* archive_base,
    const char* directory_base)
{
    guint archive_index, directory_index;
    for (archive_index = 0, directory_index = 0;
         archive_index < archive->len && directory_index < directory->len;
         ++archive_index, ++directory_index)
    {
        const char* archive_file = archive->pdata[archive_index];
        const char* directory_file = directory->pdata[directory_index];
        if (!strcmp(archive_file, directory_file) &&
            check_file_for_modifications(archive_file, archive_url,
                archive_base, directory_base))
        {
            printf("M %s\n", (const char*)archive->pdata[archive_index]);
        }

        // If there's a file in the archive but not the directory, it's been
        // deleted
        else if (!check_list_for_file(archive_file,
                directory->pdata + directory_index,
                directory->len - directory_index))
        {
            printf("D %s\n", (const char*)archive->pdata[archive_index]);
            archive_index += 1;
        }

        // If there's a file in the directory but not in the archive, it's been
        // added
        else if (!check_list_for_file(directory_file,
                archive->pdata + archive_index,
                archive->len - archive_index))
        {
            printf("A %s\n", (const char*)directory->pdata[directory_index]);
        }
    }

    // If there are remaining files in either list, we know what to do.
    for (guint i = directory_index; i < directory->len; ++i) {
        printf("A %s\n", (const char*)directory->pdata[i]);
    }

    for (guint i = archive_index; i < archive->len; ++i) {
        printf("D %s\n", (const char*)archive->pdata[i]);
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
    static const char* archive_base = "./";
    remove_matching_entry(archive, archive_base);
    trim_prefix_from_entries(archive, archive_base);

    remove_matching_entry(directory, live_volume->mountpoint);
    size_t mountpoint_length = strlen(live_volume->mountpoint);
    if ('/' != live_volume->mountpoint[mountpoint_length - 1]) {
        // Have to add that terminating '/'
        char* mountpoint = string_append_new(string_new(
                live_volume->mountpoint), &(char){'/'});
        trim_prefix_from_entries(directory, mountpoint);
        free(mountpoint);
    } else {
        trim_prefix_from_entries(directory, live_volume->mountpoint);
    }

    // Sort the lists
    g_ptr_array_sort(archive, (GCompareFunc)g_ascii_strcasecmp);
    g_ptr_array_sort(directory, (GCompareFunc)g_ascii_strcasecmp);
    result = diff_directory_from_archive(archive, directory,
        volume.archive.url, archive_base, live_volume->mountpoint);
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
