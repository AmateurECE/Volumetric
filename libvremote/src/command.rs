///////////////////////////////////////////////////////////////////////////////
// NAME:            command.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     High-level, common information for all commands.
//
// CREATED:         10/10/2021
//
// LAST EDITED:     10/28/2021
//
// Copyright 2021, Ethan D. Twardy
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

pub mod init;
pub mod add;
pub mod status;
pub mod commit;
pub mod generate;
pub mod deploy;
pub mod external;

use std::io;
use std::default::Default;
use crate::repository::Settings;
use crate::RemoteImpl;

const VOLUMETRIC_FILE: &'static str = "volumetric.yaml";
const DATA_DIR: &'static str      = ".volumetric";
const LOCK_FILE: &'static str     = ".volumetric/lock";
const SETTINGS_FILE: &'static str = ".volumetric/settings";
const HISTORY_FILE: &'static str  = ".volumetric/history";
const OBJECTS_DIR: &'static str   = ".volumetric/objects";
const CHANGES_DIR: &'static str   = ".volumetric/changes";
const STAGING_DIR: &'static str   = ".volumetric/staging";
const TMP_DIR: &'static str       = ".volumetric/tmp";

pub fn load_settings<R: RemoteImpl>(transport: &mut R) -> io::Result<Settings>
{
    match transport.get_file(&SETTINGS_FILE) {
        Ok(mut reader) => Settings::from(reader.as_mut()),
        Err(_) => Ok(Settings::default()),
    }
}

pub fn write_settings<R: RemoteImpl>(transport: &mut R, settings: &Settings) ->
    io::Result<usize>
{
    transport.put_file(&SETTINGS_FILE, &settings.to_string().as_bytes())
}

///////////////////////////////////////////////////////////////////////////////
