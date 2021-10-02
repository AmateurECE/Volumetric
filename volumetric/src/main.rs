///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the volumetric program
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/01/2021
////

use std::error::Error;

extern crate libvremote;
use libvremote::{remote_type, RemoteSpec};

fn main() -> Result<(), Box<dyn Error>> {
    match remote_type("file:///usr/share/remote".to_string()).unwrap() {
        RemoteSpec::File(s) => println!("path: {}", s.get_path()),
    };
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
