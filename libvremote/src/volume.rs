///////////////////////////////////////////////////////////////////////////////
// NAME:            volume.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implements logic around a volume.
//
// CREATED:         10/09/2021
//
// LAST EDITED:     10/10/2021
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
use std::path::Path;
use std::process;

use libvruntime::OciRuntime;
use serde::{Serialize, Deserialize};

use crate::hash;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Volume {
    pub name: String,
    pub hash: String,
}

impl Volume {
    pub fn new(name: &str) -> Volume {
        Volume {
            name: name.to_owned(),
            hash: "/dev/null".to_string(),
        }
    }

    // Stage a snapshot of a volume for commit. Return the hash.
    pub fn stage<P: AsRef<Path>>(
        &mut self, driver: Box<dyn OciRuntime>, tmp_object: P
    ) -> io::Result<()> {
        let host_path = driver.get_volume_host_path(&self.name).unwrap();
        let status = process::Command::new("tar")
            .args(["czvf", tmp_object.as_ref().to_str().unwrap(),
                   "-C", host_path.to_str().unwrap(), "."])
            .status()
            .expect("Error running tar");
        if !status.success() {
            return Err(io::Error::from_raw_os_error(
                status.code().unwrap()));
        }

        self.hash = hash::sha256sum(tmp_object)?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
