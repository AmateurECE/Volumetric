///////////////////////////////////////////////////////////////////////////////
// NAME:            init.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to encapsulate init subcommand.
//
// CREATED:         10/10/2021
//
// LAST EDITED:     10/31/2021
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

use std::io;
use std::convert::TryInto;
use std::error::Error;
use std::marker::PhantomData;

use clap::{App, Arg, ArgMatches, SubCommand};
use libvremote::Persistent;
use libvremote::remote::WriteRemote;
use libvremote::repository::{self, RepositoryStructure};

pub fn usage() -> App<'static, 'static> {
    SubCommand::with_name("init")
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
             .short("u"))
}

pub fn init<P, R>(
    remote: R, matches: &ArgMatches, finder: RepositoryStructure
) -> Result<(), Box<dyn Error>>
where
    P: io::Read + io::Write,
    R: WriteRemote<P>,
{
    let mut settings = repository::Settings::default();
    if let Some(oci_runtime) = matches.value_of("oci-runtime") {
        settings.oci_runtime = oci_runtime.try_into()?;
    }

    if let Some(remote_uri) = matches.value_of("remote-uri") {
        settings.remote_uri = Some(remote_uri.to_string());
    }

    let mut initializer = Init::new(remote, finder);
    initializer.init(settings)
}

struct Init<P: io::Read + io::Write, R: WriteRemote<P>> {
    remote: R,
    pathfinder: RepositoryStructure,
    phantom: PhantomData<P>,
}

impl<P: io::Read + io::Write, R: WriteRemote<P>> Init<P, R> {
    pub fn new(remote: R, pathfinder: RepositoryStructure) -> Init<P, R> {
        Init { remote, pathfinder, phantom: PhantomData }
    }

    // Populate all the initial artifacts
    pub fn init(&mut self, settings: repository::Settings) ->
        Result<(), Box<dyn Error>>
    {
        self.remote.create_dir(self.pathfinder.get_data())?;
        settings.store(&mut self.remote.upload_file(
            self.pathfinder.get_settings())?)?;

        let lock = repository::Lock::default();
        lock.store(&mut self.remote.upload_file(self.pathfinder.get_lock())?)?;

        let history = repository::History::default();
        history.store(&mut self.remote.upload_file(
            self.pathfinder.get_history())?)?;

        self.remote.create_dir(self.pathfinder.get_objects())?;
        self.remote.create_dir(self.pathfinder.get_changes())?;
        let stage = self.pathfinder.get_stage();
        self.remote.create_dir(stage.get_stage())?;
        self.remote.create_dir(stage.get_tmp())?;
        self.remote.create_dir(stage.get_lock())?;
        self.remote.create_dir(stage.get_objects())?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
