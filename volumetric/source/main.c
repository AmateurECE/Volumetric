///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the application.
//
// CREATED:         01/16/2022
//
// LAST EDITED:     01/16/2022
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

#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <config.h>

static const char* VOLUME_DIRECTORY = CONFIG_VOLUME_DIRECTORY;

static void print_error(const char* message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    fprintf(stderr, ": %s\n", strerror(errno));
}

static int load_entry(FILE* input_file) {
    // TODO
    return -ENOSYS;
}

static int load_configurations(const char* path, int (*load_entry)(FILE*)) {
    struct dirent* entry = NULL;
    DIR* directory = opendir(path);
    if (NULL == directory) {
        print_error("cannot open directory %s", path);
        return errno;
    }

    int result = 0;
    while (NULL != (entry = readdir(directory))) {
        FILE* input_file = fopen(entry->d_name, "rb");
        if (NULL == input_file) {
            print_error("error opening file %s", entry->d_name);
        }

        result = load_entry(input_file);
        fclose(input_file);
        if (0 != result) {
            break;
        }
    }

    closedir(directory);
    return result;
}

int main(int argc, char** argv) {
    load_configurations(VOLUME_DIRECTORY, load_entry);
}

///////////////////////////////////////////////////////////////////////////////
