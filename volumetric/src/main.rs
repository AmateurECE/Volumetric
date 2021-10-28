///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the volumetric program.
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/28/2021
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

use std::convert::TryInto;
use std::error::Error;
use std::io;
use std::io::Write;

extern crate clap;
use clap::{App, AppSettings, Arg, SubCommand};

extern crate libvremote;
use libvremote::{
    remote_type, RemoteSpec, FileRemote, Init, Add, Status, Commit, Generate,
    Deploy, External, Compressor, Stage, Lock, ObjectStore,
};

extern crate libvruntime;

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
        .subcommand(SubCommand::with_name("config")
                    .about("View or set configuration of a repository")
                    .arg(Arg::with_name("list")
                         .short("l")
                         .long("list"))
                    .arg(Arg::with_name("option")
                         .help("Option to set"))
                    .arg(Arg::with_name("value")
                         .help("Value to set for option")))
        .subcommand(SubCommand::with_name("external")
                    .about("Track snapshot of a volume hosted elsewhere")
                    .subcommand(SubCommand::with_name("add")
                                .about("Add an external volume")
                                .arg(Arg::with_name("volume")
                                     .help("Volume name")
                                     .required(true))
                                .arg(Arg::with_name("hash")
                                     .help("Hash of the snapshot")
                                     .required(true))
                                .arg(Arg::with_name("uri")
                                     .help("URI of the volume")
                                     .required(true))))
        .get_matches();

    let uri = matches.value_of("uri").unwrap_or(".");
    let mut remote = match remote_type(uri.to_string()).unwrap() {
        RemoteSpec::File(e) => FileRemote::new(e)?,
    };
    let mut settings = libvremote::load_settings(&mut remote)?;

    if let Some(matches) = matches.subcommand_matches("init") {
        let oci_runtime = matches.value_of("oci-runtime").unwrap_or("docker");
        settings.oci_runtime = oci_runtime.try_into()?;
        if let Some(remote_uri) = matches.value_of("remote-uri") {
            settings.remote_uri = Some(remote_uri.to_string());
        }
        let mut initializer = Init::new(remote, settings);
        initializer.init()?;
    }

    else if let Some(matches) = matches.subcommand_matches("add") {
        let volume = matches.value_of("volume")
            .expect("Must provide a volume name!");
        let mut adder = Add::new(
            remote, settings, Compressor::new(), Lock::new());
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
        let mut deployer = Deploy::new(remote, Compressor::new());
        deployer.deploy(&file)?;
    }

    else if let Some(matches) = matches.subcommand_matches("config") {
        let stdout = io::stdout();
        let mut stdout = stdout.lock();
        let option = matches.value_of("option");
        if matches.is_present("list") {
            if let Some(option) = option {
                // Print option
                let value = settings.get(&option)?;
                write!(stdout, "{}: {}\n", &option, &value)?;
            } else {
                // Print entire configuration
                settings.iter()
                    .for_each(|(k, v)| write!(stdout, "{}: {}\n", &k, &v)
                              .unwrap());
            }
        } else {
            // Set option to value (or unset)
            settings.set(option.unwrap(), matches.value_of("value"))?;
            // Write settings
            libvremote::write_settings(&mut remote, &settings)?;
        }
    }

    else if let Some(matches) = matches.subcommand_matches("external") {
        if let Some(sub_matches) = matches.subcommand_matches("add") {
            // external add subcommand
            let volume = sub_matches.value_of("volume").unwrap();
            let hash = sub_matches.value_of("hash").unwrap();
            let uri = sub_matches.value_of("uri").unwrap();
            let mut externalizer = External::new(
                remote, Stage::new(Lock::new(), ObjectStore::new()), settings);
            externalizer.add(&volume, &hash, &uri)?;
        }
    }
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
