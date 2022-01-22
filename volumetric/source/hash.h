///////////////////////////////////////////////////////////////////////////////
// NAME:            hash.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Interface (to libcrypto, really) to calculate various
//                  hashes of memory blocks.
//
// CREATED:         01/21/2022
//
// LAST EDITED:     01/22/2022
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

#ifndef VOLUMETRIC_HASH_H
#define VOLUMETRIC_HASH_H

#include <stdbool.h>
#include <stddef.h>

typedef enum FileHashType {
    FILE_HASH_TYPE_INVALID,
    FILE_HASH_TYPE_MD5,
} FileHashType;

typedef struct FileHash {
    FileHashType type;
    char* hash_string;
} FileHash;

FileHashType string_to_file_hash_type(const char* string);

bool check_hash_of_memory(void* buffer, size_t length, const FileHash* hash);

#endif // VOLUMETRIC_HASH_H

///////////////////////////////////////////////////////////////////////////////
