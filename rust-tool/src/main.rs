///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Volumetric. Tool for container volume management.
//
// CREATED:         01/07/2022
//
// LAST EDITED:     01/08/2022
//
// Copyright 2022, Ethan D. Twardy
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

use clap::{Parser, Subcommand};

mod configuration;
mod add;
mod init;
mod external;
mod volume;

use configuration::{load, store};

#[derive(Parser, Debug)]
#[clap(about, version, author)]
struct Args {
    #[clap(subcommand)]
    command: Commands,

    #[clap(short, long, default_value = "volumetric.yaml")]
    file: String,
}

#[derive(Subcommand, Debug)]
enum Commands {
    Init,

    Add {
        #[clap(subcommand)]
        kind: VolumeKinds,
    },
}

#[derive(Subcommand, Debug)]
enum VolumeKinds {
    External { name: String, url: String, revision: String },
}

fn main() -> Result<(), Box<dyn Error>> {
    let args = Args::parse();

    match &args.command {
        Commands::Init => {
            init::init(&args.file);
        },

        Commands::Add { kind } => {
            match &kind {
                VolumeKinds::External { name, url, revision } => {
                    let mut config = load(&args.file)?;
                    add::add_external(&mut config, &name, &url, &revision);
                    store(&args.file, &config)?;
                }
            }
        }
    }

    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
