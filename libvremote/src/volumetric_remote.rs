///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implements functionality for talking to a remote repository
//
// CREATED:         10/03/2021
//
// LAST EDITED:     10/03/2021
////

use std::collections::HashMap;
use std::error::Error;

use handlebars::Handlebars;

// extern crate yaml_rust;
// use yaml_rust::{YamlEmitter, YamlLoader};

extern crate libvruntime;
use libvruntime::OciRuntimeType;

use crate::{RemoteImpl, REPOSITORY_VERSION};

const INITIAL_LOCK: &'static str = "";

const INITIAL_SETTINGS: &'static str = "\
version: '{{VERSION}}'
oci-runtime: '{{oci_runtime}}'
";

const INITIAL_HISTORY: &'static str = "";

pub struct VolumetricRemote<R: RemoteImpl> {
    transport: R,
    oci_runtime: libvruntime::OciRuntimeType,
}

impl<R: RemoteImpl> VolumetricRemote<R> {
    pub fn new(transport: R) -> VolumetricRemote<R> {
        VolumetricRemote {
            transport,
            // TODO: Detect OCI Runtime here?
            oci_runtime: OciRuntimeType::Docker,
        }
    }

    pub fn set_runtime(&mut self, oci_runtime: OciRuntimeType) {
        self.oci_runtime = oci_runtime;
    }

    pub fn init(&mut self) -> Result<(), Box<dyn Error>> {
        let mut handlebars = Handlebars::new();
        handlebars.register_template_string("settings", INITIAL_SETTINGS)?;

        // Populate all the initial artifacts
        self.transport.put_file("lock", &mut INITIAL_LOCK.as_bytes())?;

        let mut settings = HashMap::new();
        settings.insert("oci_runtime", self.oci_runtime.to_string());
        settings.insert("VERSION", REPOSITORY_VERSION.to_string());
        let settings = handlebars.render("settings", &settings)?;
        self.transport.put_file("settings", &settings.as_bytes())?;

        self.transport.put_file("history", &mut INITIAL_HISTORY.as_bytes())?;
        self.transport.create_dir("objects")?;
        self.transport.create_dir("changes")?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
