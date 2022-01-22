###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Project file.
#
# CREATED:          01/16/2022
#
# LAST EDITED:      01/22/2022
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

project('volumetric', 'c', version: '0.1.1')

config_data = configuration_data({
  'version': meson.project_version(),
  'configuration_file': get_option('configuration_file'),
})
configure_file(input: 'config.h.in', output: 'config.h',
               configuration: config_data)

libgobiserde = dependency('libgobiserde')
libglib = dependency('glib-2.0')
libcurl = dependency('libcurl')
libjson_c = dependency('json-c')
libcrypto = dependency('libcrypto')
libarchive = dependency('libarchive')

executable(
  'volumetric',
  sources: [
    'volumetric/main.c',
    'volumetric/configuration.c',
    'volumetric/volume.c',
    'volumetric/versioning.c',
    'volumetric/docker.c',
    'volumetric/file.c',
    'volumetric/hash.c',
    'volumetric/extract.c',
  ],
  c_args: [
    '-frecord-gcc-switches', '--std=c18', '-D_XOPEN_SOURCE=500', '-O0', '-g',
  ],
  dependencies: [
    libgobiserde, libglib, libcurl, libjson_c, libcrypto, libarchive,
  ],
  install: true,
)

# Install systemd service
install_data('volumetric.service', install_dir: 'lib/systemd/system')

###############################################################################
