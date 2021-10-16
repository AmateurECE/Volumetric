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
use std::iter::FromIterator;
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
    from: fn(&mut M, &M),
}

pub const SETTINGS_SETTERS: &'static [Setter<Settings>] = &[
    Setter {
        key: "version",
        setter: |map, val| Ok(map.version = val.to_string()),
        getter: |map| map.version.clone(),
        from: |dest, source| dest.version = source.version.clone(),
    },
    Setter {
        key: "oci_runtime",
        setter: |map, val| Ok(map.oci_runtime = val.try_into()?),
        getter: |map| map.oci_runtime.to_string(),
        from: |dest, source| dest.oci_runtime = source.oci_runtime,
    },
    Setter {
        key: "remote_uri",
        setter: |map, val| Ok(map.remote_uri = Some(val.to_owned())),
        getter: |map| match &map.remote_uri {
            Some(uri) => uri.to_owned(),
            None => "~".to_owned(),
        },
        from: |dest, source| dest.remote_uri = source.remote_uri.clone(),
    },
];

impl Settings {
    pub fn from(reader: &mut dyn io::Read) -> io::Result<Settings> {
        let val: serde_yaml::Value = serde_yaml::from_reader(reader).unwrap();
        Settings::from_yaml(&val.as_mapping().unwrap())
    }

    pub fn from_yaml(serial: &serde_yaml::Mapping) -> io::Result<Settings> {
        let mut settings = Settings::default();
        serial.iter()
            .for_each(|(k, v)| settings.set(
                k.as_str().unwrap(), v.as_str()).unwrap());
        Ok(settings)
    }

    pub fn set(mut self: &mut Self, key: &str, value: Option<&str>) ->
        io::Result<()>
    {
        let setting = SETTINGS_SETTERS.iter()
            .find(|k| k.key == key)
            .ok_or(io::Error::new(
                io::ErrorKind::Other, format!("Unknown variant: {}", key)))?;
        if let Some(value) = value {
            (setting.setter)(&mut self, value)
        } else {
            Ok((setting.from)(&mut self, &Settings::default()))
        }
    }

    pub fn get(&self, key: &str) -> io::Result<String> {
        let setting = SETTINGS_SETTERS.iter()
            .find(|k| k.key == key)
            .ok_or(io::Error::new(
                io::ErrorKind::Other, format!("Unknown variant: {}", key)))?;
        Ok((setting.getter)(&self))
    }

    pub fn iter<'a>(&'a self) -> impl Iterator<Item = (String, String)> + 'a {
        SETTINGS_SETTERS.iter()
            .map(move |setter| (setter.key.to_owned(), (setter.getter)(&self)))
    }
}

impl ToString for Settings {
    fn to_string(&self) -> String {
        let defaults = serde_yaml::to_value(&Settings::default()).unwrap();
        let current = serde_yaml::to_value(&self).unwrap();
        let current = current.as_mapping().unwrap().clone().into_iter().filter(
            |(k, v)| v != defaults.as_mapping().unwrap().get(k).unwrap()
                || k == "version"
        );
        serde_yaml::to_string(&serde_yaml::Mapping::from_iter(current))
            .unwrap()
    }
}

///////////////////////////////////////////////////////////////////////////////
