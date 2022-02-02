///////////////////////////////////////////////////////////////////////////////
// NAME:            string.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     String utility implementations.
//
// CREATED:         01/29/2022
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <volumetric-diff/string.h>

char* string_append_new(char* string, const char* immutable) {
    size_t length = strlen(string) + strlen(immutable) + 1;
    char* new_string = malloc(length);
    assert(NULL != new_string);
    memset(new_string, 0, length);
    strcat(new_string, string);
    strcat(new_string, immutable);
    free(string);
    return new_string;
}

char* string_join_new(char* string, char join, const char* immutable) {
    size_t current_length = strlen(string);
    size_t length = current_length + 1 + strlen(immutable) + 1;
    char* new_string = malloc(length);
    assert(NULL != new_string);
    memset(new_string, 0, length);
    strcat(new_string, string);
    new_string[current_length] = join;
    strcat(new_string, immutable);
    free(string);
    return new_string;
}

char* string_new(const char* immutable) {
    size_t length = strlen(immutable) + 1;
    char* string = malloc(length);
    assert(NULL != string);
    memset(string, 0, length);
    strcat(string, immutable);
    return string;
}

///////////////////////////////////////////////////////////////////////////////
