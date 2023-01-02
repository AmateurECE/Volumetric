///////////////////////////////////////////////////////////////////////////////
// NAME:            archive.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the file extraction interface.
//
// CREATED:         01/22/2022
//
// LAST EDITED:     01/01/2023
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <archive.h>
#include <archive_entry.h>

#include <volumetric/archive.h>
#include <volumetric/file.h>

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static int copy_data(struct archive* reader, struct archive* writer) {
    int result = 0;
    const void* buff = NULL;
    size_t size = 0;
    la_int64_t offset = 0;

    for (;;) {
        result = archive_read_data_block(reader, &buff, &size, &offset);
        if (result == ARCHIVE_EOF)
            return ARCHIVE_OK;
        if (result < ARCHIVE_OK)
            return result;

        result = archive_write_data_block(writer, buff, size, offset);
        if (result < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(writer));
            return result;
        }
    }
}

static void prepend_directory_path(const char* directory,
                                   struct archive_entry* entry) {
    const char* current_path = archive_entry_pathname(entry);
    size_t directory_length = strlen(directory);
    size_t path_length = directory_length + 1 + strlen(current_path);
    char* new_path = malloc(path_length + 1);
    assert(NULL != new_path);

    memset(new_path, 0, path_length + 1);
    strcat(new_path, directory);
    new_path[directory_length] = '/';
    strcat(new_path, current_path);
    new_path[path_length] = '\0';

    archive_entry_set_pathname(entry, new_path);
    // clang-tidy complains about the memory leak related to the passing of
    // ownership of the malloc'd string "new_path" to the struct archive_entry.
    // Unfortunately, the benefit of fixing this leak doesn't seem justified
    // based on the effort. So, we disable the lint.
    // NOLINTNEXTLINE(clang-analyzer-unix.Malloc)
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

void archive_extract_to_disk_universal(const FileContents* file,
                                       const char* location) {
    /* Select which attributes we want to restore. */
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS |
                ARCHIVE_EXTRACT_OWNER;

    struct archive* read_archive = archive_read_new();
    archive_read_support_format_all(read_archive);
    archive_read_support_filter_all(read_archive);
    struct archive* extractor = archive_write_disk_new();
    archive_write_disk_set_options(extractor, flags);
    archive_write_disk_set_standard_lookup(extractor);

    int result =
        archive_read_open_memory(read_archive, file->contents, file->size);
    assert(0 == result);

    struct archive_entry* entry = NULL;
    for (;;) {

        result = archive_read_next_header(read_archive, &entry);
        if (result == ARCHIVE_EOF)
            break;
        if (result < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(read_archive));
        assert(ARCHIVE_WARN <= result);

        prepend_directory_path(location, entry);

        result = archive_write_header(extractor, entry);
        if (result < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(extractor));
        } else if (archive_entry_size(entry) > 0) {
            result = copy_data(read_archive, extractor);
            if (result < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(extractor));
            assert(ARCHIVE_WARN <= result);
        }

        result = archive_write_finish_entry(extractor);
        if (result < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(extractor));
        assert(ARCHIVE_WARN <= result);
    }

    archive_read_close(read_archive);
    archive_read_free(read_archive);
    archive_write_close(extractor);
    archive_write_free(extractor);
}

///////////////////////////////////////////////////////////////////////////////
