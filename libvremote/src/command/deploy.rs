///////////////////////////////////////////////////////////////////////////////
// NAME:            deploy.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Command for deploying a configuration (using
//                  "volumetric.yaml") to an OCI Runtime.
//
// CREATED:         10/10/2021
//
// LAST EDITED:     10/26/2021
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
use std::collections::HashMap;
use std::fmt;
use std::io;
use std::path::{Path, PathBuf};
use serde::{Serialize, Deserialize};
use serde_yaml;

use crate::RemoteImpl;
use crate::settings::Settings;
use crate::command::OBJECTS_DIR;
use crate::compressor::Compressor;
use crate::volume::Volume;
use crate::variant_error::VariantError;

#[derive(Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub enum DeploymentPolicy {
    Overwrite,
    DoNotOverwrite,
}

impl TryFrom<&str> for DeploymentPolicy {
    type Error = VariantError;
    fn try_from(value: &str) -> Result<Self, Self::Error> {
        match &value.to_lowercase().as_str() {
            &"overwrite" => Ok(DeploymentPolicy::Overwrite),
            &"donotoverwrite" => Ok(DeploymentPolicy::DoNotOverwrite),
            &_ => Err(VariantError::new(value)),
        }
    }
}

impl fmt::Display for DeploymentPolicy {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        fmt::Debug::fmt(self, f)
    }
}

pub struct Deploy<R: RemoteImpl> {
    transport: R,
    compressor: Compressor,
}

impl<R: RemoteImpl> Deploy<R> {
    pub fn new(transport: R, compressor: Compressor) -> Deploy<R> {
        Deploy { transport, compressor }
    }

    fn read_configuration<P: AsRef<Path>>(&mut self, name: P) ->
        serde_yaml::Result<(Settings, HashMap<String, Volume>)>
    {
        let mut conf: serde_yaml::Value = serde_yaml::from_reader(
            self.transport.get_file(name).unwrap())?;
        let volumes = conf.as_mapping_mut().unwrap()
            .remove(&serde_yaml::to_value("volumes")?)
            .unwrap();
        Ok((
            Settings::try_from(&conf).unwrap(),
            serde_yaml::from_value::<HashMap<String, Volume>>(volumes)?,
        ))
    }

    pub fn deploy<P: AsRef<Path>>(&mut self, configuration: P) ->
        io::Result<()>
    {
        // Open configuration, read it
        let (settings, volumes) = self.read_configuration(configuration)
            .unwrap();
        let driver = libvruntime::get_oci_runtime(settings.oci_runtime);

        // For each volume in the configuration: If the volume exists in the
        //   runtime, delete it. Create the volume, and populate it.
        for (_, volume) in volumes {
            if driver.volume_exists(&volume.name).unwrap() {
                match settings.deployment_policy {
                    DeploymentPolicy::Overwrite =>
                        driver.remove_volume(&volume.name).unwrap(),
                    DeploymentPolicy::DoNotOverwrite => continue,
                }
            }

            driver.create_volume(&volume.name).unwrap();
            let image_path = PathBuf::from(OBJECTS_DIR).join(&volume.hash);
            self.compressor.restore(
                driver.get_volume_host_path(&volume.name).unwrap(),
                self.transport.get_path(&image_path)
            )?;
        }

        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
