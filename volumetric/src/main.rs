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
use clap::{App, AppSettings, Arg, SubCommand};

extern crate libvremote;
use libvremote::{VolumetricRemote, remote_type, RemoteSpec, FileRemote};

extern crate libvruntime;
use libvruntime::get_oci_runtime;

fn do_init(uri: String, oci_runtime: String) -> Result<(), Box<dyn Error>> {
    let mut remote = match remote_type(uri).unwrap() {
        RemoteSpec::File(e) => VolumetricRemote::new(FileRemote::new(e)?),
    };
    remote.set_runtime(get_oci_runtime(oci_runtime)?);
    remote.init()?;
    Ok(())
}

fn do_add(_volume: String) -> Result<(), Box<dyn Error>> {
    unimplemented!()
}

fn main() -> Result<(), Box<dyn Error>> {
    let matches = App::new("Volumetric")
        .version("0.1.0")
        .author("Ethan D. Twardy <ethan.twardy@gmail.com>")
        .about("Version control for OCI Volumes")
        .settings(&[AppSettings::SubcommandRequiredElseHelp])
        .subcommand(SubCommand::with_name("init")
                    .about("Initialize a repository in the current directory")
                    .arg(Arg::with_name("uri")
                         .help("URI of a directory to initialize a repo in."))
                    .arg(Arg::with_name("oci-runtime")
                         .takes_value(true)
                         .long("oci-runtime")
                         .short("r")))
        .subcommand(SubCommand::with_name("add")
                    .about("Track changes to a volume in the OCI Runtime")
                    .arg(Arg::with_name("volume")
                         .help("Name of a persistent volume in the runtime")))
        .get_matches();

    if let Some(matches) = matches.subcommand_matches("init") {
        let uri = matches.value_of("uri").unwrap_or(".");
        let oci_runtime = matches.value_of("oci-runtime").unwrap_or("docker");
        do_init(uri.to_string(), oci_runtime.to_string())?;
    } else if let Some(matches) = matches.subcommand_matches("add") {
        let volume = matches.value_of("volume")
            .expect("Must provide a volume name!");
        do_add(volume.to_string())?;
    }
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
