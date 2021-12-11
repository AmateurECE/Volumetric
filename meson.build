###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Top level makefile for the project
#
# CREATED:          12/10/2021
#
# LAST EDITED:      12/11/2021
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

libcurl = dependency('libcurl')
libyaml = dependency('yaml-0.1')

libvolumetric_sources = files([
  'libvolumetric/configuration.c',
])

libvolumetric = library(
  'volumetric',
  sources: libvolumetric_sources,
  dependencies: [libyaml],
  version: meson.project_version(),
  install: true,
)

volumetric_remote_sources = files([
  'volumetric-remote/volumetric-remote.c',
])

volumetric_remote = executable(
  'volumetric-remote',
  sources: volumetric_remote_sources,
  dependencies: [libcurl],
  link_with: [libvolumetric],
)

###############################################################################
