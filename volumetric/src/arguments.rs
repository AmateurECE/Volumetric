///////////////////////////////////////////////////////////////////////////////
// NAME:            arguments.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Argument parsing logic
//
// CREATED:         10/28/2021
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

extern crate clap;
use clap::{App, AppSettings, Arg, ArgMatches, SubCommand};

pub fn get_arguments() -> ArgMatches<'static> {
    App::new("Volumetric")
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
        .get_matches()
}

///////////////////////////////////////////////////////////////////////////////
