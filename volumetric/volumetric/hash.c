///////////////////////////////////////////////////////////////////////////////
// NAME:            hash.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the hash algorithms.
//
// CREATED:         01/22/2022
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

#include <assert.h>
#include <string.h>

#include <openssl/evp.h>

#include <volumetric/hash.h>

typedef bool HashCheckerFn(void*, size_t, unsigned char*, size_t);

static void convert_hex_string_to_bytes(const char* string,
    unsigned char** byte_array, size_t* length)
{
    size_t byte_array_length = strlen(string) / 2;
    unsigned char* result = malloc(byte_array_length);
    assert(NULL != result);

    const char* index = string;
    for (size_t i = 0; i < *length; index += 2, ++i) {
        sscanf(index, "%2hhx", &result[i]);
    }

    *length = byte_array_length;
    *byte_array = malloc(*length);
}

static bool check_md5_hash(void* buffer, size_t length, unsigned char* hash,
    size_t hash_length)
{
    const EVP_MD* digest= EVP_get_digestbyname("MD5");
    assert(NULL != digest);

    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, digest, NULL);
    EVP_DigestUpdate(context, buffer, length);

    unsigned char md_value[EVP_MAX_MD_SIZE] = {0};
    unsigned int md_length = 0;
    EVP_DigestFinal_ex(context, md_value, &md_length);
    EVP_MD_CTX_free(context);

    size_t comparison_length = hash_length;
    if (md_length < comparison_length) {
        comparison_length = md_length;
    }

    bool result = !!strncmp((const char*)hash, (const char*)md_value,
        comparison_length);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

FileHashType string_to_file_hash_type(const char* string) {
    if (!strcmp("md5", string)) {
        return FILE_HASH_TYPE_MD5;
    } else {
        fprintf(stderr, "Unknown hash type: %s\n", string);
        return FILE_HASH_TYPE_INVALID;
    }
}

bool check_hash_of_memory(void* buffer, size_t length, const FileHash* hash) {
    HashCheckerFn* hash_checker = NULL;
    switch (hash->type) {
    case FILE_HASH_TYPE_MD5: hash_checker = check_md5_hash; break;
    default:
        assert(0); // Programmer's error.
    }

    unsigned char* hash_bytes = NULL;
    size_t hash_length = 0;
    convert_hex_string_to_bytes(hash->hash_string, &hash_bytes, &hash_length);
    bool result = hash_checker(buffer, length, hash_bytes, hash_length);
    free(hash_bytes);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
