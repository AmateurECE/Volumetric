///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the volumetric program
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/03/2021
////

use std::error::Error;

extern crate clap;
use clap::{App, Arg, SubCommand};

extern crate libvremote;
use libvremote::{VolumetricRemote, remote_type, RemoteSpec, FileRemote};

fn main() -> Result<(), Box<dyn Error>> {
    let matches = App::new("Volumetric")
        .version("0.1.0")
        .author("Ethan D. Twardy <ethan.twardy@gmail.com>")
        .about("Version control for OCI Volumes")
        .subcommand(SubCommand::with_name("init")
                    .about("Initialize a repository in the current directory")
                    .arg(Arg::with_name("uri")
                         .help("URI of a directory to initialize a repo in.")))
        .get_matches();

    if let Some(matches) = matches.subcommand_matches("init") {
        let uri = matches.value_of("uri").unwrap();
        let mut remote = match remote_type(uri.to_string()).unwrap() {
            RemoteSpec::File(e) => VolumetricRemote::new(FileRemote::new(e)?),
        };
        remote.init()?;
    }
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
