///////////////////////////////////////////////////////////////////////////////
// NAME:            settings.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for handling Settings: repository configuration.
//
// CREATED:         10/12/2021
//
// LAST EDITED:     10/16/2021
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

use std::convert::TryInto;
use std::io;
use std::string::ToString;
use serde::{Serialize, Deserialize};
use libvruntime::OciRuntimeType;

use crate::REPOSITORY_VERSION;

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct Settings {
    pub version: String,
    pub oci_runtime: OciRuntimeType,
    pub remote_uri: Option<String>,
}

impl Default for Settings {
    fn default() -> Settings {
        Settings {
            version: REPOSITORY_VERSION.to_string(),
            oci_runtime: OciRuntimeType::Docker,
            remote_uri: None,
        }
    }
}

pub struct Setter<'a, M: Sync> {
    key: &'a str,
    setter: fn(&mut M, &str) -> io::Result<()>,
    getter: fn(&M) -> String,
}

pub const SETTINGS_SETTERS: &'static [Setter<Settings>] = &[
    Setter {
        key: "version",
        setter: |map, val| Ok(map.version = val.to_string()),
        getter: |map| map.version.clone(),
    },
    Setter {
        key: "oci_runtime",
        setter: |map, val| Ok(map.oci_runtime = val.try_into()?),
        getter: |map| map.oci_runtime.to_string(),
    },
];

impl Settings {
    pub fn from(reader: &mut dyn io::Read) -> io::Result<Settings> {
        Ok(serde_yaml::from_reader(reader).unwrap())
    }

    pub fn serialize(&self) -> io::Result<String> {
        Ok(serde_yaml::to_string(&self).unwrap())
    }

    pub fn set(mut self: &mut Self, key: &str, value: &str) -> io::Result<()> {
        let setting = SETTINGS_SETTERS.iter()
            .find(|k| k.key == key)
            .ok_or(io::Error::new(
                io::ErrorKind::Other, "Unknown variant!"))?;
        (setting.setter)(&mut self, value)
    }

    pub fn get(&self, key: &str) -> io::Result<String> {
        let setting = SETTINGS_SETTERS.iter()
            .find(|k| k.key == key)
            .ok_or(io::Error::new(
                io::ErrorKind::Other, "Unknown variant!"))?;
        Ok((setting.getter)(&self))
    }
}

///////////////////////////////////////////////////////////////////////////////
