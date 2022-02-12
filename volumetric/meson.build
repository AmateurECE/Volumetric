###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Project file.
#
# CREATED:          01/16/2022
#
# LAST EDITED:      02/12/2022
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

libglib = dependency('glib-2.0')
libserdec = dependency('libserdec')

executable(
  'volumetric',
  sources: [
    'main.c',
    'versioning.c',
  ],
  c_args: [
    '-frecord-gcc-switches', '--std=c18', '-D_XOPEN_SOURCE=500', '-O0', '-g',
  ],
  include_directories: ['../libvolumetric'],
  dependencies: [libglib, libserdec],
  link_with: [libvolumetric],
  install: true,
)

# Install systemd service
install_data('volumetric.service', install_dir: get_option('systemd_unitdir'))

###############################################################################
