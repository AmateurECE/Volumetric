///////////////////////////////////////////////////////////////////////////////
// NAME:            file.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Useful utilities for loading files from various filesystems
//
// CREATED:         01/21/2022
//
// LAST EDITED:     01/21/2022
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

#ifndef VOLUMETRIC_FILE_H
#define VOLUMETRIC_FILE_H

#include <sys/types.h>

// Intentionally opaque data structure hiding implementation details.
typedef struct MemoryMappedFile MemoryMappedFile;

// Opaque struct to represent a file in memory. This could be a file from a
// mounted filesystem, or a file downloaded over HTTP, etc.
typedef struct FileContents {
    void* contents;
    size_t size;
    MemoryMappedFile* file;
} FileContents;

// Open the file at <path> and fill the memory buffer with its contents
int file_contents_init(FileContents* file, const char* path);

// Release memory held by this object.
void file_contents_release(FileContents* file);

#endif // VOLUMETRIC_FILE_H

///////////////////////////////////////////////////////////////////////////////
