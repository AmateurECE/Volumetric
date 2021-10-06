///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implements functionality for talking to a remote repository
//
// CREATED:         10/04/2021
//
// LAST EDITED:     10/05/2021
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

use std::collections::HashMap;
use std::error::Error;

use serde::{Serialize, Deserialize};
use serde_yaml;
use libvruntime::{OciRuntimeType, OciRuntime, Docker, Podman};
use crate::{RemoteImpl, REPOSITORY_VERSION};

const INITIAL_HISTORY: &'static str = "";

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct LockFile {
    pub volumes: HashMap<String, String>
}

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct SettingsFile {
    pub version: String,
    pub oci_runtime: OciRuntimeType,
}

impl Default for SettingsFile {
    fn default() -> SettingsFile {
        SettingsFile {
            version: REPOSITORY_VERSION.to_string(),
            oci_runtime: OciRuntimeType::Docker,
        }
    }
}

pub struct VolumetricRemote<R: RemoteImpl> {
    transport: R,
    settings: SettingsFile,
}

impl<R: RemoteImpl> VolumetricRemote<R> {
    pub fn new(mut transport: R) -> VolumetricRemote<R> {
        let settings: SettingsFile = match transport.get_file("settings") {
            Ok(reader) => serde_yaml::from_reader(reader)
                .expect("Could not read settings file from repository"),
            Err(_) => SettingsFile::default(),
        };

        VolumetricRemote { transport, settings, }
    }

    pub fn set_runtime(&mut self, oci_runtime: OciRuntimeType) {
        self.settings.oci_runtime = oci_runtime;
    }

    // Populate all the initial artifacts
    pub fn init(&mut self) -> Result<(), Box<dyn Error>> {
        let lock = LockFile { volumes: HashMap::new() };
        let lock = serde_yaml::to_string(&lock)?;
        self.transport.put_file("lock", &mut lock.as_bytes())?;

        let lock_orig = "";
        self.transport.put_file("lock.orig", &mut lock_orig.as_bytes())?;

        let settings = serde_yaml::to_string(&self.settings)?;
        self.transport.put_file("settings", &settings.as_bytes())?;

        self.transport.put_file("history", &mut INITIAL_HISTORY.as_bytes())?;
        self.transport.create_dir("objects")?;
        self.transport.create_dir("changes")?;
        Ok(())
    }

    fn get_driver(&self) -> Box<dyn OciRuntime> {
        match self.settings.oci_runtime {
            OciRuntimeType::Docker => Box::new(Docker::new()),
            OciRuntimeType::Podman => Box::new(Podman::new()),
        }
    }

    // Add a volume to the lock file.
    pub fn add(&mut self, volume: String) -> Result<(), Box<dyn Error>> {
        let driver = self.get_driver();
        if driver.volume_exists(&volume)? {
            let mut lock: LockFile = serde_yaml::from_reader(
                self.transport.get_file("lock")?)?;
            lock.volumes.insert(volume, "/dev/null".to_string());
            let lock = serde_yaml::to_string(&lock)?;
            self.transport.put_file("lock", &lock.as_bytes())?;
        }

        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
