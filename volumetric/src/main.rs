///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the volumetric program.
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/04/2021
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

extern crate clap;
use clap::{App, AppSettings, Arg, SubCommand};

extern crate libvremote;
use libvremote::{
    VolumetricRemote, remote_type, RemoteSpec, FileRemote, RemoteImpl,
};

extern crate libvruntime;
use libvruntime::get_oci_runtime;

fn do_init<R: RemoteImpl>(
    mut remote: VolumetricRemote<R>, oci_runtime: String
) -> Result<(), Box<dyn Error>> {
    remote.set_runtime(get_oci_runtime(oci_runtime)?);
    remote.init()?;
    Ok(())
}

fn do_add<R: RemoteImpl>(
    mut remote: VolumetricRemote<R>, volume: String
) -> Result<(), Box<dyn Error>> {
    remote.add(volume)
}

fn main() -> Result<(), Box<dyn Error>> {
    let matches = App::new("Volumetric")
        .version("0.1.0")
        .author("Ethan D. Twardy <ethan.twardy@gmail.com>")
        .about("Version control for OCI Volumes")
        .settings(&[AppSettings::SubcommandRequiredElseHelp])
        .arg(Arg::with_name("uri")
             .help("URI of a repository"))
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
                    .arg(Arg::with_name("uri")
                         .help("URI of the repository"))
                    .arg(Arg::with_name("volume")
                         .help("Name of a persistent volume in the runtime")))
        .get_matches();

    let uri = matches.value_of("uri").unwrap_or(".");
    let remote = match remote_type(uri.to_string()).unwrap() {
        RemoteSpec::File(e) => VolumetricRemote::new(FileRemote::new(e)?),
    };

    if let Some(matches) = matches.subcommand_matches("init") {
        let oci_runtime = matches.value_of("oci-runtime").unwrap_or("docker");
        do_init(remote, oci_runtime.to_string())?;
    } else if let Some(matches) = matches.subcommand_matches("add") {
        let volume = matches.value_of("volume")
            .expect("Must provide a volume name!");
        do_add(remote, volume.to_string())?;
    }
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
