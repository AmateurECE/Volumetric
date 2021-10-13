///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the volumetric program.
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/12/2021
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

extern crate clap;
use clap::{App, AppSettings, Arg, SubCommand};

extern crate libvremote;
use libvremote::{
    SettingsFile, remote_type, RemoteSpec, FileRemote, Init, Add, Status,
    Commit, Generate, Deploy,
};

extern crate libvruntime;
use libvruntime::get_oci_runtime_type;

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
                         .short("r"))
                    .arg(Arg::with_name("remote-uri")
                         .help("Remote URI that the repository is reached at")
                         .takes_value(true)
                         .long("remote-uri")
                         .short("u")))
        .subcommand(SubCommand::with_name("add")
                    .about("Track changes to a volume in the OCI Runtime")
                    .arg(Arg::with_name("volume")
                         .help("Name of a persistent volume in the runtime")))
        .subcommand(SubCommand::with_name("status")
                    .about("Show status of the volumes repository"))
        .subcommand(SubCommand::with_name("commit")
                    .about("Commit staged changes"))
        .subcommand(SubCommand::with_name("generate")
                    .about("Generate a volumetric.yaml from the repository"))
        .subcommand(SubCommand::with_name("deploy")
                    .about("Deploy a volumetric configuration to the runtime")
                    .arg(Arg::with_name("file")
                         .help("Volumetric configuration (volumetric.yaml)")
                         .takes_value(true)
                         .long("file")
                         .short("f")))
        .get_matches();

    let uri = matches.value_of("uri").unwrap_or(".");
    let mut remote = match remote_type(uri.to_string()).unwrap() {
        RemoteSpec::File(e) => FileRemote::new(e)?,
    };
    let mut settings = SettingsFile::from(&mut remote);

    if let Some(matches) = matches.subcommand_matches("init") {
        let oci_runtime = matches.value_of("oci-runtime").unwrap_or("docker");
        let remote_uri = matches.value_of("remote-uri").unwrap_or(&uri);
        settings.set_runtime(get_oci_runtime_type(oci_runtime.to_string())?);
        settings.set_remote_uri(remote_uri.to_string());
        let mut initializer = Init::new(remote, settings);
        initializer.init()?;
    }

    else if let Some(matches) = matches.subcommand_matches("add") {
        let volume = matches.value_of("volume")
            .expect("Must provide a volume name!");
        let mut adder = Add::new(remote, settings);
        adder.add(volume.to_string())?;
    }

    else if matches.subcommand_name().unwrap() == "status" {
        let stdout = io::stdout();
        let mut printer = Status::new(remote);
        printer.status(stdout.lock())?;
    }

    else if matches.subcommand_name().unwrap() == "commit" {
        let mut committer = Commit::new(remote);
        committer.commit()?;
    }

    else if matches.subcommand_name().unwrap() == "generate" {
        let mut generator = Generate::new(remote, settings);
        generator.generate()?;
    }

    else if let Some(matches) = matches.subcommand_matches("deploy") {
        let file = matches.value_of("file").unwrap_or("volumetric.yaml");
        let mut deployer = Deploy::new(remote);
        deployer.deploy(&file)?;
    }
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
