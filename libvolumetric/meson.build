###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Build script for the convenience library.
#
# CREATED:          01/26/2022
#
# LAST EDITED:      12/30/2022
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

config_data = configuration_data({
  'version': meson.project_version(),
  'configuration_file': get_option('configuration_file'),
})
configure_file(input: 'config.h.in', output: 'config.h',
               configuration: config_data)

libserdec = dependency('libserdec')
libglib = dependency('glib-2.0')
libcurl = dependency('libcurl')
libjson_c = dependency('json-c')
libcrypto = dependency('libcrypto')
libarchive = dependency('libarchive')

libvolumetric = library(
  'volumetric',
  sources: [
    'volumetric/archive.c',
    'volumetric/file.c',
    'volumetric/hash.c',
    'volumetric/volume.c',
    'volumetric/configuration.c',
    'volumetric/project-file.c',
    'volumetric/directory.c',
    'volumetric/string-handling.c',

    'volumetric/docker/proxy.c',
    'volumetric/docker/volume.c',
    'volumetric/docker/container.c',

    'volumetric/volume/archive/commit.c',
    'volumetric/volume/archive/deser.c',
    'volumetric/volume/archive/status.c',
    'volumetric/volume/archive/versioning.c',
    'volumetric/volume/archive/lock-file.c',
  ],
  dependencies: [
    libserdec, libglib, libcurl, libjson_c, libcrypto, libarchive,
  ],
  soversion: meson.project_version(),
  install: true,
)

###############################################################################
