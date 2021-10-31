///////////////////////////////////////////////////////////////////////////////
// NAME:            settings.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic for handling Settings: repository configuration.
//
// CREATED:         10/12/2021
//
// LAST EDITED:     10/31/2021
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

use std::fmt;
use std::io;
use std::string::ToString;
use serde::{ser, de, ser::SerializeMap};
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

impl ser::Serialize for Settings {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where S: ser::Serializer {
        let defaults = Settings::default();
        let filtered = self.iter().filter(
            |(key, value)| value != &defaults.get(&key).unwrap()
                || key == "version"
        ).collect::<Vec<(String, String)>>();
        let mut map = serializer.serialize_map(Some(filtered.len()))?;
        for (key, value) in filtered {
            map.serialize_entry(&key, &value)?;
        }
        map.end()
    }
}

struct SettingsVisitor;
impl<'de> de::Visitor<'de> for SettingsVisitor {
    type Value = Settings;
    fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        formatter.write_str("a map")
    }

    fn visit_map<A>(self, mut map: A) -> Result<Self::Value, A::Error>
    where A: de::MapAccess<'de> {
        let mut settings = Settings::default();
        while let Some((key, value)) = map.next_entry::<&str, &str>()? {
            settings.set(key, Some(value)).unwrap();
        }
        Ok(settings)
    }
}

impl<'de> de::Deserialize<'de> for Settings {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where D: de::Deserializer<'de> {
        deserializer.deserialize_map(SettingsVisitor)
    }
}

impl Persistent for Settings {
    fn load(target: &mut dyn io::Read) -> io::Result<Self> {
        serde_yaml::from_reader::<&mut dyn io::Read, Settings>(target)
            .map_err(|_| io::Error::new(
                io::ErrorKind::Other, "deserialization error"))
    }

    fn store(&self, target: &mut dyn io::Write) -> io::Result<()> {
        serde_yaml::to_writer::<&mut dyn io::Write, Settings>(target, &self)
            .map_err(|_| io::Error::new(
                io::ErrorKind::Other, "serialization error"))
    }
}

///////////////////////////////////////////////////////////////////////////////
