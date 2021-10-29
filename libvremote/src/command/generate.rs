///////////////////////////////////////////////////////////////////////////////
// NAME:            generate.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Generate volumetric.yaml, which is meant to be committed to
//                  external source control and can be used to clone/deploy a
//                  setup on any OCI Runtime.
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

use std::convert::TryFrom;
use std::error::Error;
use std::collections::HashMap;
use serde_yaml;

use crate::RemoteImpl;
use crate::volume::Volume;
use crate::repository::Settings;
use crate::command::{LOCK_FILE, VOLUMETRIC_FILE};

pub struct Generate<R: RemoteImpl> {
    transport: R,
    settings: Settings,
}

impl<R: RemoteImpl> Generate<R> {
    pub fn new(transport: R, settings: Settings) -> Generate<R> {
        Generate { transport, settings }
    }

    pub fn generate(&mut self) -> Result<(), Box<dyn Error>> {
        // TODO: This should be a local-only file.
        let mut object = serde_yaml::Value::try_from(&self.settings).unwrap();
        let volumes: HashMap<String, Volume> = serde_yaml::from_reader(
            self.transport.get_file(&LOCK_FILE)?)?;
        object.as_mapping_mut().unwrap()
            .insert(serde_yaml::to_value("volumes")?,
                    serde_yaml::to_value(volumes)?);
        let object = serde_yaml::to_string(&object)?;
        self.transport.put_file(&VOLUMETRIC_FILE, &object.as_bytes())?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
