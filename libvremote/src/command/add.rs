///////////////////////////////////////////////////////////////////////////////
// NAME:            add.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Adds resources to the staging area
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

use std::collections::HashMap;
use std::error::Error;
use std::io;
use std::path;

use crate::RemoteImpl;
use crate::volume::Volume;
use crate::command::{STAGING_DIR, LOCK_FILE, TMP_DIR, SettingsFile};

pub struct Add<R: RemoteImpl> {
    transport: R,
    settings: SettingsFile,
}

impl<R: RemoteImpl> Add<R> {
    pub fn new(transport: R, settings: SettingsFile) -> Add<R> {
        Add { transport, settings }
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
        let driver = libvruntime::get_oci_runtime(self.settings.oci_runtime);
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
}

///////////////////////////////////////////////////////////////////////////////
