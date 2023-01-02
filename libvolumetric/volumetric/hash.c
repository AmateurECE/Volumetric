///////////////////////////////////////////////////////////////////////////////
// NAME:            hash.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the hash algorithms.
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
#include <errno.h>
#include <string.h>

#include <openssl/evp.h>

#include <volumetric/hash.h>

static void convert_hex_string_to_bytes(const char* string,
                                        unsigned char** byte_array,
                                        size_t* length) {
    size_t byte_array_length = strlen(string) / 2;
    unsigned char* result = malloc(byte_array_length);
    assert(NULL != result);

    const char* index = string;
    for (size_t i = 0; i < byte_array_length; index += 2, ++i) {
        sscanf(index, "%2hhx", &result[i]);
    }

    *length = byte_array_length;
    *byte_array = result;
}

static void get_md5_hash(void* buffer, size_t length, unsigned char* hash_out,
                         unsigned int* hash_length) {
    assert(EVP_MAX_MD_SIZE <= *hash_length);
    const EVP_MD* digest = EVP_get_digestbyname("MD5");
    assert(NULL != digest);

    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, digest, NULL);
    EVP_DigestUpdate(context, buffer, length);

    EVP_DigestFinal_ex(context, hash_out, hash_length);
    EVP_MD_CTX_free(context);
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

FileHashType file_hash_type_from_string(const char* string) {
    // TODO: As we support more hash types, it would be better to convert this
    // to lowercase instead of comparing both versions.
    if (!strcmp("md5", string) || !strcmp("MD5", string)) {
        return FILE_HASH_TYPE_MD5;
    } else {
        fprintf(stderr, "Unknown hash type: %s\n", string);
        return FILE_HASH_TYPE_INVALID;
    }
}

const char* file_hash_type_to_string(FileHashType hash_type) {
    switch (hash_type) {
    case FILE_HASH_TYPE_MD5:
        return "md5";
    case FILE_HASH_TYPE_INVALID:
        return "invalid";
    default:
        assert(0); // Programmer's error
    }
}

FileHash* file_hash_of_buffer(FileHashType hash_type, void* buffer,
                              size_t length) {
    FileHash* file_hash = malloc(sizeof(FileHash));
    if (NULL == file_hash) {
        return NULL;
    }
    memset(file_hash, 0, sizeof(FileHash));

    if (FILE_HASH_TYPE_MD5 == hash_type) {
        file_hash->hash_type = hash_type;
        unsigned int hash_length = EVP_MAX_MD_SIZE;
        file_hash->hash_string = malloc(hash_length);
        if (NULL == file_hash->hash_string) {
            free(file_hash);
            return NULL;
        }
        memset(file_hash->hash_string, 0, hash_length);

        get_md5_hash(buffer, length, (unsigned char*)file_hash->hash_string,
                     &hash_length);
        file_hash->hash_length = (size_t)hash_length;
    } else {
        fprintf(stderr, "%s:%d: Unknown FileHashType", __FILE__, __LINE__);
        free(file_hash);
        return NULL;
    }

    return file_hash;
}

FileHash* file_hash_from_string(FileHashType type, const char* hex_string) {
    FileHash* file_hash = malloc(sizeof(FileHash));
    if (NULL == file_hash) {
        return NULL;
    }
    memset(file_hash, 0, sizeof(FileHash));

    file_hash->hash_type = type;
    convert_hex_string_to_bytes(hex_string,
                                (unsigned char**)&file_hash->hash_string,
                                &file_hash->hash_length);
    return file_hash;
}

char* file_hash_to_string(const FileHash* hash) {
    size_t string_length = (hash->hash_length * 2) + 1;
    char* string = malloc(string_length);
    if (NULL == string) {
        return NULL;
    }

    int result = 0;
    for (size_t i = 0; i < hash->hash_length; ++i) {
        result = snprintf(&string[i * 2], 3, "%02hhx", hash->hash_string[i]);
        assert(2 == result); // Or else something has gone horribly wrong.
    }

    return string;
}

void file_hash_free(FileHash* hash) {
    free(hash->hash_string);
    free(hash);
}

bool file_hash_equal(const FileHash* first, const FileHash* second) {
    if (first->hash_type != second->hash_type) {
        return false;
    }

    if (first->hash_length != second->hash_length) {
        return false;
    }

    return !memcmp(first->hash_string, second->hash_string,
                   first->hash_length);
}

///////////////////////////////////////////////////////////////////////////////
