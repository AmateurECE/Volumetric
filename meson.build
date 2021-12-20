###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Top level makefile for the project
#
# CREATED:          12/10/2021
#
# LAST EDITED:      12/19/2021
#
# Copyright 2021, Ethan D. Twardy
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

project(
  'volumetric', 'c',
  version: '0.1.0',
  default_options: [
    'buildtype=debug',
    'warning_level=3',
    'c_std=c18',
  ]
)
add_project_arguments('-frecord-gcc-switches', language: 'c')

config_data = configuration_data({
  'version': meson.project_version(),
  'name': meson.project_name(),
})
configure_file(input: 'config.h.in', output: 'config.h',
               configuration: config_data)

libcurl = dependency('libcurl')
libglib = dependency('glib-2.0')
libyaml = dependency('yaml-0.1')

volumetric_sources = files([
  'volumetric/argparse.c',
  'volumetric/volumetric.c',
  'volumetric/add-packaged.c',
  'volumetric/configuration.c',
])

volumetric = executable(
  'volumetric',
  sources: volumetric_sources,
  dependencies : [libglib],
)

###############################################################################
