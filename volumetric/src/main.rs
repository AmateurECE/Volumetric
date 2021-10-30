///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the volumetric program.
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/29/2021
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

extern crate libvremote;
use libvremote::{
    remote_type, RemoteSpec, FileRemote, Init, Add, Status, Commit, Generate,
    Deploy, External, Compressor, Stage, Lock, ObjectStore,
};

extern crate libvruntime;

mod arguments;
mod subcommands;

fn dispatch_read<P, R>(remote: R, matches: clap::ArgMatches) ->
    Result<(), Box<dyn Error>>
where
    P: io::Read,
    R: ReadRemote<R>,
{
    let (subcommand, arg_matches) = matches.subcommand();
    match subcommand {
        &"init" => subcommands::init(remote, arg_matches),
        &"add" => Ok(()),
        &"status" => Ok(()),
        &"commit" => Ok(()),
        &"generate" => Ok(()),
        &"deploy" => Ok(()),
        &"config" => Ok(()),
        &"external" => Ok(()),
        &_ => libvruntime::VariantError::new(),
    }
}

fn main() -> Result<(), Box<dyn Error>> {
    let matches = arguments::get_arguments();
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
