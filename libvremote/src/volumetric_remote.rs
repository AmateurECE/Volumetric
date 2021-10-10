///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implements functionality for talking to a remote repository
//
// CREATED:         10/04/2021
//
// LAST EDITED:     10/09/2021
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
use std::io;
use std::path;

use serde::{Serialize, Deserialize};
use serde_yaml;
use libvruntime::{OciRuntimeType, OciRuntime, Docker, Podman};
use crate::{RemoteImpl, REPOSITORY_VERSION};

mod volume;
use volume::Volume;

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
        self.transport.create_dir(&DATA_DIR)?;
        let settings = serde_yaml::to_string(&self.settings)?;
        self.transport.put_file(&SETTINGS_FILE, &settings.as_bytes())?;
        self.transport.put_file(
            &LOCK_FILE, &serde_yaml::to_string(
                &HashMap::<String, Volume>::new())?.as_bytes())?;
        self.transport.put_file(&HISTORY_FILE, "".as_bytes())?;

        self.transport.create_dir(&OBJECTS_DIR)?;
        self.transport.create_dir(&CHANGES_DIR)?;
        self.transport.create_dir(&STAGING_DIR)?;
        self.transport.create_dir(&TMP_DIR)?;

        let staging_objects = path::PathBuf::from(STAGING_DIR).join("objects");
        self.transport.create_dir(&staging_objects.to_str().unwrap())?;

        Ok(())
    }

    fn get_driver(&self) -> Box<dyn OciRuntime> {
        match self.settings.oci_runtime {
            OciRuntimeType::Docker => Box::new(Docker::new()),
            OciRuntimeType::Podman => Box::new(Podman::new()),
        }
    }

    fn lock_staged_volume(&mut self, volume: Volume) -> io::Result<()> {
        let staging_lock = path::PathBuf::from(STAGING_DIR).join("lock");
        let mut volumes: HashMap<String, Volume>
            = match self.transport.get_file(&staging_lock) {
                Ok(file) => serde_yaml::from_reader(file).unwrap(),
                // If the file does not exist...
                Err(e) => match e.kind() {
                    io::ErrorKind::NotFound => {
                        // Attempt to copy it from DATA_DIR
                        self.transport.copy(&LOCK_FILE, &staging_lock)?;
                        serde_yaml::from_reader(
                            self.transport.get_file(&staging_lock)?).unwrap()
                    },
                    _ => return Err(e),
                },
            };

        volumes.insert(volume.name.to_owned(), volume);
        let volumes = serde_yaml::to_string(&volumes).unwrap();
        self.transport.put_file(&staging_lock, &volumes.as_bytes())?;
        Ok(())
    }

    // Stage a volume for commit.
    pub fn add(&mut self, volume: String) -> Result<(), Box<dyn Error>> {
        let driver = self.get_driver();
        if !driver.volume_exists(&volume)? {
            return Err(Box::new(io::Error::new(
                io::ErrorKind::NotFound, "No such volume")));
        }

        // TODO: Cannot support remote transports with this
        let tmp_object = path::PathBuf::from(TMP_DIR)
            .join(volume.to_owned() + ".tar.gz");
        let mut volume = Volume::new(&volume);
        volume.stage(driver, &self.transport.get_path(&tmp_object))?;
        let snapshot_object = path::PathBuf::from(STAGING_DIR)
            .join("objects").join(&volume.hash);
        self.transport.rename(&tmp_object, &snapshot_object)?;

        // Update .volumetric/staging/lock with the hash of the new volume.
        self.lock_staged_volume(volume)?;
        Ok(())
    }

    fn print_status<W: io::Write>(
        &self, volumes: Vec<(String, String)>, mut writer: W) -> io::Result<()>
    {
        let padding = volumes.iter()
            .map(|(k, _)| k.len())
            .reduce(|l, m| l.max(m))
            .unwrap();
        let padding = padding + (8 - (padding % 8)) + 8;
        for (volume, status) in volumes {
            write!(writer,
                   "{:padding$}{}\n",
                   &volume, &status, padding=padding,
            )?;
        }
        Ok(())
    }

    fn status_staging(&mut self, status: &mut Vec<(String, String)>) ->
        io::Result<()>
    {
        let staging_lock = path::PathBuf::from(STAGING_DIR).join("lock");
        let staged: HashMap<String, Volume>
            = match self.transport.get_file(&staging_lock) {
                Ok(file) => serde_yaml::from_reader(file).unwrap(),
                Err(e) => match e.kind() {
                    io::ErrorKind::NotFound => return Ok(()),
                    _ => return Err(e),
                },
            };

        let volumes: HashMap<String, Volume> = serde_yaml::from_reader(
            self.transport.get_file(&LOCK_FILE)?).unwrap();

        staged.iter()
            .filter(|(k, _)| !volumes.contains_key(k.as_str()))
            .for_each(|(k, _)| status.push((k.clone(), "Added".to_string())));
        Ok(())
    }

    // TODO: Maybe can get a speedup with async?
    // TODO: Show progress?
    // TODO: Maybe show sizes of snapshots?
    // Write status information about the repository to writer
    pub fn status<W: io::Write>(
        &mut self, out: W) -> Result<(), Box<dyn Error>>
    {
        let mut status = Vec::<(String, String)>::new();

        // 1. Volumes in the staging area
        self.status_staging(&mut status)?;

        // 2. Volumes showing changes either from the staging area or the lock
        //    in the runtime (this will take a long time to calculate, so we
        //    should enable/disable this with a flag).
        // 3. Commits behind master

        self.print_status(status, out)?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
