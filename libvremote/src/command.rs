///////////////////////////////////////////////////////////////////////////////
// NAME:            command.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     High-level, common information for all commands.
//
// CREATED:         10/10/2021
//
// LAST EDITED:     10/12/2021
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

use serde::{Serialize, Deserialize};
use libvruntime::OciRuntimeType;

use crate::{RemoteImpl, REPOSITORY_VERSION};

pub mod init;
pub mod add;
pub mod status;
pub mod commit;
pub mod generate;
pub mod deploy;

const VOLUMETRIC_FILE: &'static str = "volumetric.yaml";
const DATA_DIR: &'static str      = ".volumetric";
const LOCK_FILE: &'static str     = ".volumetric/lock";
const SETTINGS_FILE: &'static str = ".volumetric/settings";
const HISTORY_FILE: &'static str  = ".volumetric/history";
const OBJECTS_DIR: &'static str   = ".volumetric/objects";
const CHANGES_DIR: &'static str   = ".volumetric/changes";
const STAGING_DIR: &'static str   = ".volumetric/staging";
const TMP_DIR: &'static str       = ".volumetric/tmp";

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct SettingsFile {
    pub version: String,
    pub oci_runtime: OciRuntimeType,
    pub remote_uri: Option<String>,
}

impl Default for SettingsFile {
    fn default() -> SettingsFile {
        SettingsFile {
            version: REPOSITORY_VERSION.to_string(),
            oci_runtime: OciRuntimeType::Docker,
            remote_uri: None,
        }
    }
}

impl SettingsFile {
    pub fn from<R: RemoteImpl>(transport: &mut R) -> SettingsFile {
        let settings = match transport.get_file(&SETTINGS_FILE) {
            Ok(reader) => serde_yaml::from_reader(reader)
                .expect("Could not read settings file from repository"),
            Err(_) => SettingsFile::default(),
        };

        settings
    }

    pub fn set_runtime(&mut self, oci_runtime: OciRuntimeType) {
        self.oci_runtime = oci_runtime;
    }

    pub fn set_remote_uri(&mut self, remote_uri: String) {
        self.remote_uri = Some(remote_uri);
    }
}

///////////////////////////////////////////////////////////////////////////////
