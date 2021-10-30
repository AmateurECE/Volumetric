///////////////////////////////////////////////////////////////////////////////
// NAME:            settings.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for handling Settings: repository configuration.
//
// CREATED:         10/12/2021
//
// LAST EDITED:     10/30/2021
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
use std::string::ToString;
use serde::{Serialize, Serializer, Deserialize, Deserializer};
use libvruntime::OciRuntimeType;
use libvruntime::error::VariantError;

use crate::REPOSITORY_VERSION;
use crate::persistence::Persistent;

mod deployment_policy;
mod setters;

pub use deployment_policy::DeploymentPolicy;
use setters::SETTINGS_SETTERS;

#[derive(Debug, PartialEq)]
pub struct Settings {
    pub version: String,
    pub oci_runtime: OciRuntimeType,
    pub remote_uri: Option<String>,
    pub deployment_policy: DeploymentPolicy,
}

impl Settings {
    pub fn set(mut self: &mut Self, key: &str, value: Option<&str>) ->
        Result<(), VariantError>
    {
        let setting = SETTINGS_SETTERS.iter()
            .find(|k| k.key == key)
            .ok_or_else(|| VariantError::new(key))?;
        if let Some(value) = value {
            (setting.setter)(&mut self, value)
        } else {
            Ok((setting.from)(&mut self, &Settings::default()))
        }
    }

    pub fn get(&self, key: &str) -> Result<String, VariantError> {
        let setting = SETTINGS_SETTERS.iter()
            .find(|k| k.key == key)
            .ok_or_else(|| VariantError::new(key))?;
        Ok((setting.getter)(&self))
    }

    pub fn iter<'a>(&'a self) -> impl Iterator<Item = (String, String)> + 'a {
        SETTINGS_SETTERS.iter()
            .map(move |setter| (setter.key.to_owned(), (setter.getter)(&self)))
    }
}

impl Default for Settings {
    fn default() -> Settings {
        Settings {
            version: REPOSITORY_VERSION.to_string(),
            oci_runtime: OciRuntimeType::Docker,
            remote_uri: None,
            deployment_policy: DeploymentPolicy::NoOverwrite,
        }
    }
}

impl Serialize for Settings {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where S: Serializer
    {
        // TODO: Implementation
        unimplemented!()
    }
}

impl<'de> Deserialize<'de> for Settings {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where D: Deserializer<'de>
    {
        // TODO: Implementation
        unimplemented!()
    }
}

impl Persistent for Settings {
    fn load(target: &mut dyn io::Read) -> io::Result<Self> {
        // TODO: Implementation
        unimplemented!()
    }

    fn store(&self, target: &mut dyn io::Write) -> io::Result<()> {
        // TODO: Implementation
        unimplemented!()
    }
}

///////////////////////////////////////////////////////////////////////////////
