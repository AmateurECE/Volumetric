///////////////////////////////////////////////////////////////////////////////
// NAME:            setters.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Homogeneous interfaces for setting/gettings config.
//
// CREATED:         10/30/2021
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

use std::convert::TryInto;
use crate::repository::settings::Settings;
use libvruntime::error::VariantError;

pub struct Setter<'a, M: Sync> {
    pub key: &'a str,
    pub setter: fn(&mut M, &str) -> Result<(), VariantError>,
    pub getter: fn(&M) -> String,
    pub from: fn(&mut M, &M),
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
    Setter {
        key: "deployment_policy",
        setter: |map, val| Ok(map.deployment_policy = val.try_into()?),
        getter: |map| map.deployment_policy.to_string(),
        from: |dest, source| dest.deployment_policy = source.deployment_policy,
    },
];

///////////////////////////////////////////////////////////////////////////////
