///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Impl for the VolumetricRemote struct.
//
// CREATED:         10/03/2021
//
// LAST EDITED:     10/03/2021
////

use std::error::Error;

// extern crate yaml_rust;
// use yaml_rust::{YamlEmitter, YamlLoader};

extern crate const_format;

use crate::{RemoteImpl, VolumetricRemote, REPOSITORY_VERSION};

const INITIAL_LOCK: &'static str = "";

const INITIAL_SETTINGS: &'static str = const_format::formatcp!("\
version: '{VERSION}'
", VERSION = REPOSITORY_VERSION);

const INITIAL_HISTORY: &'static str = "";

impl<R: RemoteImpl> VolumetricRemote<R> {
    pub fn new(transport: R) -> VolumetricRemote<R> {
        VolumetricRemote { transport }
    }

    pub fn init(&mut self) -> Result<(), Box<dyn Error>> {
        // Populate all the initial artifacts
        self.transport.put_file("lock", &mut INITIAL_LOCK.as_bytes())?;
        self.transport.put_file("settings", &mut INITIAL_SETTINGS.as_bytes())?;
        self.transport.put_file("history", &mut INITIAL_HISTORY.as_bytes())?;
        self.transport.create_dir("objects")?;
        self.transport.create_dir("changes")?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
