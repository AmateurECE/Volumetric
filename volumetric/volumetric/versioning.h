///////////////////////////////////////////////////////////////////////////////
// NAME:            versioning.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to version the volumes passed in.
//
// CREATED:         01/17/2022
//
// LAST EDITED:     02/11/2022
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

#ifndef VOLUMETRIC_VERSIONING_H
#define VOLUMETRIC_VERSIONING_H

typedef struct Volume Volume;
typedef struct Docker Docker;
int version_volume(Volume* volume, Docker* docker);

#endif // VOLUMETRIC_VERSIONING_H

///////////////////////////////////////////////////////////////////////////////
