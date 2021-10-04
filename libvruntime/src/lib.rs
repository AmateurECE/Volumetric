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

#[derive(Debug)]
pub enum OciRuntime {
    Docker,
}

impl fmt::Display for OciRuntime {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        fmt::Debug::fmt(self, f)
    }
}

pub fn get_oci_runtime(runtime_str: String) -> io::Result<OciRuntime> {
    match runtime_str.to_lowercase().as_str() {
        "docker" => Ok(OciRuntime::Docker),

        // Error case
        &_ => Err(io::Error::new(
            io::ErrorKind::Other,
            format!("Invalid runtime: {}", runtime_str))
        ),
    }
}

///////////////////////////////////////////////////////////////////////////////
