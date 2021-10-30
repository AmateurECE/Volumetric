///////////////////////////////////////////////////////////////////////////////
// NAME:            lib.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     This crate is an abstraction layer for different OCI
//                  Runtimes.
//
// CREATED:         10/04/2021
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

use std::convert::TryFrom;
use std::error::Error;
use std::io;
use std::fmt;
use std::path;

use serde;
use serde::{Serialize, Deserialize};

mod docker;
mod podman;
pub mod error;

pub use docker::Docker;
pub use podman::Podman;

pub trait OciRuntime {
    fn volume_exists(&self, volume: &str) -> Result<bool, Box<dyn Error>>;
    fn get_volume_host_path(&self, volume: &str) ->
        Result<path::PathBuf, Box<dyn Error>>;
    fn remove_volume(&self, volume: &str) -> Result<(), Box<dyn Error>>;
    fn create_volume(&self, volume: &str) -> Result<(), Box<dyn Error>>;
}

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum OciRuntimeType {
    Docker,
    Podman,
}

impl TryFrom<&str> for OciRuntimeType {
    type Error = io::Error;
    fn try_from(source: &str) -> Result<Self, Self::Error> {
        match source.to_lowercase().as_str() {
            "docker" => Ok(OciRuntimeType::Docker),
            "podman" => Ok(OciRuntimeType::Podman),
            &_ => Err(io::Error::new(io::ErrorKind::Other,
                "Unknown variant!".to_string())),
        }
    }
}

impl fmt::Display for OciRuntimeType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        fmt::Debug::fmt(self, f)
    }
}

pub fn get_oci_runtime(runtime: OciRuntimeType) -> Box<dyn OciRuntime> {
    match runtime {
        OciRuntimeType::Docker => Box::new(Docker::new()),
        OciRuntimeType::Podman => Box::new(Podman::new()),
    }
}

///////////////////////////////////////////////////////////////////////////////
