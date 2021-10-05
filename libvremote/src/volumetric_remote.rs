///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implements functionality for talking to a remote repository
//
// CREATED:         10/04/2021
//
// LAST EDITED:     10/04/2021
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

use serde_yaml;
use libvruntime::OciRuntimeType;
use crate::{RemoteImpl, REPOSITORY_VERSION};

const INITIAL_LOCK: &'static str = "";
const INITIAL_HISTORY: &'static str = "";

pub struct VolumetricRemote<R: RemoteImpl> {
    transport: R,
    oci_runtime: libvruntime::OciRuntimeType,
}

impl<R: RemoteImpl> VolumetricRemote<R> {
    pub fn new(transport: R) -> VolumetricRemote<R> {
        VolumetricRemote {
            transport,
            // TODO: Detect OCI Runtime here?
            oci_runtime: OciRuntimeType::Docker,
        }
    }

    pub fn set_runtime(&mut self, oci_runtime: OciRuntimeType) {
        self.oci_runtime = oci_runtime;
    }

    pub fn init(&mut self) -> Result<(), Box<dyn Error>> {
        // Populate all the initial artifacts
        self.transport.put_file("lock", &mut INITIAL_LOCK.as_bytes())?;

        let mut settings = HashMap::new();
        settings.insert("version", REPOSITORY_VERSION.to_string());
        settings.insert("oci-runtime", self.oci_runtime.to_string());
        let settings = serde_yaml::to_string(&settings)?;
        self.transport.put_file("settings", &settings.as_bytes())?;

        self.transport.put_file("history", &mut INITIAL_HISTORY.as_bytes())?;
        self.transport.create_dir("objects")?;
        self.transport.create_dir("changes")?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
