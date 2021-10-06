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
// LAST EDITED:     10/05/2021
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

use std::error::Error;
use std::io;
use std::fmt;

mod docker;

pub use docker::Docker;

trait OciRuntime {}

#[derive(Debug, Clone, Copy)]
pub enum OciRuntimeType {
    Docker,
}

impl fmt::Display for OciRuntimeType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        fmt::Debug::fmt(self, f)
    }
}

pub fn get_oci_runtime(runtime_str: String) -> io::Result<OciRuntimeType> {
    match runtime_str.to_lowercase().as_str() {
        "docker" => Ok(OciRuntimeType::Docker),

        // Error case
        &_ => Err(io::Error::new(
            io::ErrorKind::Other,
            format!("Invalid runtime: {}", runtime_str))
        ),
    }
}

pub struct RuntimeDriver {}

impl RuntimeDriver {
    pub fn new(_type: OciRuntimeType) -> RuntimeDriver { RuntimeDriver {} }

    pub fn volume_exists(&self, _: &str) -> Result<bool, Box<dyn Error>> {
        unimplemented!()
    }
}

///////////////////////////////////////////////////////////////////////////////
