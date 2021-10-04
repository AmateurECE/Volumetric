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

///////////////////////////////////////////////////////////////////////////////
