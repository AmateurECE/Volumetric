///////////////////////////////////////////////////////////////////////////////
// NAME:            external.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to handle externally versioned volumes.
//
// CREATED:         10/18/2021
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

use std::error::Error;
use clap::ArgMatches;
use libvremote::WriteRemote;

pub fn external<P, R>(remote: R, matches: ArgMatches) ->
    Result<(), Box<dyn Error>>
where
    P: io::Read + io::Write,
    R: WriteRemote<P>
{
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

use std::error::Error;

use crate::repository::Stage;
use crate::volume::Volume;
use crate::RemoteImpl;
use crate::repository::Settings;

pub struct External<R: RemoteImpl> {
    transport: R,
    stage: Stage,
    settings: Settings,
}

impl<R: RemoteImpl> External<R> {
    pub fn new(transport: R, stage: Stage, settings: Settings) -> External<R> {
        External { transport, stage, settings }
    }

    pub fn add(&mut self, volume: &str, hash: &str, uri: &str) ->
        Result<(), Box<dyn Error>>
    {
        let volume = Volume::external(volume, hash, uri);
        self.stage.get_mut_lock().add_volume(volume)?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
