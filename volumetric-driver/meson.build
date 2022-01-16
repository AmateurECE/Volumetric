###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Build script for the Docker volume driver.
#
# CREATED:          01/09/2022
#
# LAST EDITED:      01/09/2022
#
# Copyright 2022, Ethan D. Twardy
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
###

project('volumetric-driver', 'c', version: '0.1.0')

libglib = dependency('glib-2.0')
libsoup3 = dependency('libsoup-3.0')

executable(
  'volumetric-driver',
  sources: [
    'source/main.c',
  ],
  c_args: [
    '--std=c17', '-frecord-gcc-switches',
  ],
  dependencies: [libglib, libsoup3],
)

###############################################################################
