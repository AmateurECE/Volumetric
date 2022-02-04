///////////////////////////////////////////////////////////////////////////////
// NAME:            string.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Some string utility functions.
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

#ifndef VOLUMETRIC_STRING_H
#define VOLUMETRIC_STRING_H

char* string_append_new(char* string, const char* immutable);
char* string_join_new(char* string, char join, const char* immutable);
char* string_new(const char* immutable);

#endif // VOLUMETRIC_STRING_H

///////////////////////////////////////////////////////////////////////////////
