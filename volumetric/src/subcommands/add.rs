///////////////////////////////////////////////////////////////////////////////
// NAME:            add.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the add subcommand.
//
// CREATED:         10/10/2021
//
// LAST EDITED:     10/29/2021
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

use std::io;
use std::error::Error;
use clap::ArgMatches;
use libvremote::WriteRemote;

pub fn add<P, R>(remote: R, matches: ArgMatches) -> Result<(), Box<dyn Error>>
where
    P: io::Read + io::Write,
    R: WriteRemote<P>,
{
    let volume = matches.value_of("volume")
        .expect("Must provide a volume name!");
    let mut adder = Add::new(
        remote, settings, Compressor::new(), Lock::new());
    adder.add(volume.to_string())?;
}

use std::error::Error;
use std::io;
use std::path;

use crate::RemoteImpl;
use crate::hash;
use crate::compressor::Compressor;
use crate::volume::Volume;
use crate::repository::Settings;
use crate::command::{STAGING_DIR, TMP_DIR};
use crate::repository::Lock;

pub struct Add<R: RemoteImpl> {
    transport: R,
    settings: Settings,
    compressor: Compressor,
    lock: Lock,
}

impl<R: RemoteImpl> Add<R> {
    pub fn new(
        transport: R, settings: Settings, compressor: Compressor, lock: Lock
    ) -> Add<R> {
        Add { transport, settings, compressor, lock }
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
        let mut volume = Volume::managed(&volume);
        self.compressor.stage(
            driver.get_volume_host_path(&volume.get_name())?,
            &self.transport.get_path(&tmp_object)
        )?;
        let hash = hash::sha256sum(&tmp_object)?;
        volume.set_hash(&hash);
        let snapshot_object = path::PathBuf::from(STAGING_DIR)
            .join("objects").join(&volume.get_hash());
        self.transport.rename(&tmp_object, &snapshot_object)?;

        // Update .volumetric/staging/lock with the hash of the new volume.
        self.lock.add_volume(volume)?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
