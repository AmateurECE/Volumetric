///////////////////////////////////////////////////////////////////////////////
// NAME:            lib.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     This crate is an abstraction layer for different OCI
//                  Runtimes.
//
// CREATED:         10/03/2021
//
// LAST EDITED:     10/03/2021
////

use std::io;
use std::fmt;

mod docker;

pub use docker::Docker;

trait OciRuntime {}

#[derive(Debug)]
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

///////////////////////////////////////////////////////////////////////////////
