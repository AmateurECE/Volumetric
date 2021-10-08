///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implements functionality for talking to a remote repository
//
// CREATED:         10/04/2021
//
// LAST EDITED:     10/07/2021
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
use std::fs;
use std::io;
use std::io::BufRead;
use std::process;

use serde::{Serialize, Deserialize};
use serde_yaml;
use libvruntime::{OciRuntimeType, OciRuntime, Docker, Podman};
use crate::{RemoteImpl, REPOSITORY_VERSION};

const SETTINGS_FILE: &'static str = "volumetric.yaml";
const DATA_DIR: &'static str     = ".volumetric";
const LOCK_FILE: &'static str    = ".volumetric/lock";
const HISTORY_FILE: &'static str = ".volumetric/history";
const OBJECTS_DIR: &'static str  = ".volumetric/objects";
const CHANGES_DIR: &'static str  = ".volumetric/changes";
const STAGING_DIR: &'static str  = ".volumetric/staging";

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct SettingsFile {
    pub version: String,
    pub oci_runtime: OciRuntimeType,
    pub volumes: HashMap<String, String>
}

impl Default for SettingsFile {
    fn default() -> SettingsFile {
        SettingsFile {
            version: REPOSITORY_VERSION.to_string(),
            oci_runtime: OciRuntimeType::Docker,
            volumes: HashMap::new(),
        }
    }
}

pub struct VolumetricRemote<R: RemoteImpl> {
    transport: R,
    settings: SettingsFile,
}

impl<R: RemoteImpl> VolumetricRemote<R> {
    pub fn new(mut transport: R) -> VolumetricRemote<R> {
        let settings = match transport.get_file(&SETTINGS_FILE) {
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
        let settings = serde_yaml::to_string(&self.settings)?;
        self.transport.put_file(&SETTINGS_FILE, &settings.as_bytes())?;

        self.transport.create_dir(&DATA_DIR)?;
        self.transport.create_dir(&OBJECTS_DIR)?;
        self.transport.create_dir(&CHANGES_DIR)?;
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
        if !driver.volume_exists(&volume)? {
            return Err(Box::new(io::Error::new(
                io::ErrorKind::NotFound, "No such volume")));
        }

        // 1. Set up staging area, if not already existing. (Commit command
        //    clears staging area)
        self.transport.create_dir(&STAGING_DIR)?;
        let staging_objects = STAGING_DIR.to_owned() + "/objects";
        self.transport.create_dir(&staging_objects)?;
        let tmp_dir = DATA_DIR.to_owned() + "/tmp";
        self.transport.create_dir(&tmp_dir)?;

        // 2. Zip up the volume, place in .volumetric/staging/objects/<hash>
        let host_path = driver.get_volume_host_path(&volume)?;
        let tmp_object = tmp_dir + "/" + &volume + ".tar.gz";
        let status = process::Command::new("tar")
            .args(["xzf", &tmp_object, host_path.to_str().unwrap()])
            .status()
            .expect("Error running tar");
        if !status.success() {
            return Err(Box::new(io::Error::from_raw_os_error(
                status.code().unwrap())));
        }

        let output = process::Command::new("sha256sum")
            .args([&tmp_object])
            .output()
            .expect("Error running sha256sum");
        let mut shasum = String::new();
        io::BufReader::new(io::Cursor::new(output.stdout))
            .read_line(&mut shasum)?;
        let shasum = shasum.split_whitespace().nth(0)
            .expect("Improperly formatted output from sha256sum");
        let object_file = staging_objects + "/" + shasum;
        fs::rename(&tmp_object, &object_file)?;

        // 3. Copy .volumetric/staging/lock from .volumetric/lock, if the
        //    former does not already exist
        // 4. Update .volumetric/staging/lock with the hash of the new volume.
        Ok(())
    }

    fn print_status<W: io::Write>(
        volumes: HashMap<String, String>, mut writer: W) -> io::Result<()>
    {
        let padding = volumes.iter()
            .map(|(k, _)| k.len())
            .reduce(|l, m| l.max(m))
            .expect("Error deciding padding length!");
        let padding = padding + (padding - (padding % 4)) + 4;
        for (volume, status) in volumes {
            write!(writer,
                   "{:<padding$}{}\n",
                   &volume, &status, padding=padding - volume.len()
            )?;
        }
        Ok(())
    }

    // TODO: Maybe can get a speedup with async?
    // TODO: Show progress?
    // TODO: REFACTORING. ERROR HANDLING.
    // Write status information about the repository to writer
    // Short:
    //     volume-1         Added
    //     volume-2         Changed: 3/Added: 5/Removed: 2
    // Progress (optional):
    //     volume-1         Calculating...
    pub fn status<W: io::Write>(
        &mut self, out: W) -> Result<(), Box<dyn Error>>
    {
        // Get new and old volumes
        let cur_volumes: SettingsFile = serde_yaml::from_reader(
            self.transport.get_file(&SETTINGS_FILE)?)?;
        let cur_volumes = cur_volumes.volumes;
        let old_volumes: HashMap<String, String>
            = match self.transport.get_file(&LOCK_FILE) {
                Ok(file) => {
                    let set: SettingsFile = serde_yaml::from_reader(file)?;
                    set.volumes
                },
                Err(e) => match e.kind() {
                    io::ErrorKind::NotFound => HashMap::new(),
                    _ => return Err(Box::new(e)),
                },
            };

        let mut reported_volumes = HashMap::<String, String>::new();

        // First, volumes that we've added
        for (volume, _) in cur_volumes.iter().filter(
            |(k, _)| !old_volumes.contains_key(k.as_str())) {
            reported_volumes.insert(volume.clone(), "Added".to_string());
        }

        // Second, volumes that have been removed
        for (volume, _) in old_volumes.iter().filter(
            |(k, _)| !cur_volumes.contains_key(k.as_str())) {
            reported_volumes.insert(volume.clone(), "Removed".to_string());
        }

        // Third, volumes with staged changes
        for (volume, _) in cur_volumes.iter().filter(
            |(k, v)| old_volumes.contains_key(k.as_str())
                && old_volumes.get(k.as_str()).unwrap() != *v) {
            reported_volumes.insert(volume.clone(), "Changed".to_string());
        }

        Ok(VolumetricRemote::<R>::print_status(reported_volumes, out)?)
    }
}

///////////////////////////////////////////////////////////////////////////////
