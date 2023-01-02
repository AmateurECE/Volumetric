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

#ifndef VOLUMETRIC_HASH_H
#define VOLUMETRIC_HASH_H

#include <stdbool.h>
#include <stddef.h>

typedef enum FileHashType {
    FILE_HASH_TYPE_INVALID,
    FILE_HASH_TYPE_MD5,
} FileHashType;

// The Hash string is not necessarily composed of printable characters, so
// should never be assumed to be NUL-terminated.
typedef struct FileHash {
    FileHashType hash_type;
    char* hash_string;
    size_t hash_length;
} FileHash;

// Given a string identifier, such as "md5", get the corresponding FileHashType
FileHashType file_hash_type_from_string(const char* string);

// Convert a FileHashType to a printable string.
const char* file_hash_type_to_string(FileHashType hash_type);

// Get the hash of the data in memory. Must be free'd using file_hash_free.
FileHash* file_hash_of_buffer(FileHashType hash_type, void* buffer,
                              size_t length);

// Convenience method to get a FileHash instance from a type and a hex_string.
// `type' is a string such as "md5", and `hex_string' is a string containing
// the ASCII representation of the hash bytex in hex, e.g. 'ac9687bd45'...
FileHash* file_hash_from_string(FileHashType type, const char* hex_string);

// Conveniently convert a binary hash representation to ASCII for printing. The
// string must be free'd after use.
char* file_hash_to_string(const FileHash* hash);

// Free memory consumed by a FileHash instance returned by file_hash_get.
void file_hash_free(FileHash* hash);

// Partial equality check between two file hashes
bool file_hash_equal(const FileHash* first, const FileHash* second);

#endif // VOLUMETRIC_HASH_H

///////////////////////////////////////////////////////////////////////////////
