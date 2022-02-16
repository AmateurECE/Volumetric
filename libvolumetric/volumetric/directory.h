///////////////////////////////////////////////////////////////////////////////
// NAME:            directory.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Interface for iterating through directories.
//
// CREATED:         01/29/2022
//
// LAST EDITED:     02/15/2022
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

#ifndef VOLUMETRIC_DIRECTORY_H
#define VOLUMETRIC_DIRECTORY_H

#include <dirent.h>

typedef struct DirectoryIter DirectoryIter;
typedef struct _GPtrArray GPtrArray;

typedef struct DirectoryEntry {
    struct dirent* entry;
    const char *absolute_path;
} DirectoryEntry;

DirectoryIter* directory_iter_new(const char* directory);
void directory_iter_free(DirectoryIter* iter);
DirectoryEntry* directory_iter_next(DirectoryIter* iter);

// TODO: Obviously this one is not like the others.
GPtrArray* get_file_list_for_directory(const char* directory);

#endif // VOLUMETRIC_DIRECTORY_H

///////////////////////////////////////////////////////////////////////////////
