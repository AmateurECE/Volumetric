///////////////////////////////////////////////////////////////////////////////
// NAME:            status.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     The status command prints information about the current
//                  state of the repository.
//
// CREATED:         10/10/2021
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

use std::io;
use std::error::Error;
use clap::ArgMatches;
use libvremote::ReadRemote;

pub fn status<P, R>(remote: R, matches: ArgMatches) ->
    Result<(), Box<dyn Error>>
where
    P: io::Read,
    R: ReadRemote<P>,
{
    let stdout = io::stdout();
    let mut printer = Status::new(remote);
    printer.status(stdout.lock())?;
}

use std::collections::HashMap;
use std::error::Error;
use std::io;
use std::path;

use crate::RemoteImpl;
use crate::volume::Volume;
use crate::command::{STAGING_DIR, LOCK_FILE};

pub struct Status<R: RemoteImpl> {
    transport: R,
}

impl<R: RemoteImpl> Status<R> {
    pub fn new(transport: R) -> Status<R> {
        Status { transport }
    }

    fn print_status<W: io::Write>(
        &self, volumes: Vec<(String, String)>, mut writer: W) -> io::Result<()>
    {
        let padding = volumes.iter()
            .map(|(k, _)| k.len())
            .reduce(|l, m| l.max(m))
            .unwrap();
        let padding = padding + (8 - (padding % 8)) + 8;
        for (volume, status) in volumes {
            write!(writer,
                   "{:padding$}{}\n",
                   &volume, &status, padding=padding,
            )?;
        }
        Ok(())
    }

    fn status_staging(&mut self, status: &mut Vec<(String, String)>) ->
        io::Result<()>
    {
        let staging_lock = path::PathBuf::from(STAGING_DIR).join("lock");
        let staged: HashMap<String, Volume>
            = match self.transport.get_file(&staging_lock) {
                Ok(file) => serde_yaml::from_reader(file).unwrap(),
                Err(e) => match e.kind() {
                    io::ErrorKind::NotFound => return Ok(()),
                    _ => return Err(e),
                },
            };

        let volumes: HashMap<String, Volume> = serde_yaml::from_reader(
            self.transport.get_file(&LOCK_FILE)?).unwrap();

        staged.iter()
            .filter(|(k, _)| !volumes.contains_key(k.as_str()))
            .for_each(|(k, _)| status.push((k.clone(), "Added".to_string())));
        Ok(())
    }

    // TODO: Maybe can get a speedup with async?
    // TODO: Show progress?
    // TODO: Maybe show sizes of snapshots?
    // Write status information about the repository to writer
    pub fn status<W: io::Write>(
        &mut self, out: W) -> Result<(), Box<dyn Error>>
    {
        let mut status = Vec::<(String, String)>::new();

        // 1. Volumes in the staging area
        self.status_staging(&mut status)?;

        // 2. Volumes showing changes either from the staging area or the lock
        //    in the runtime (this will take a long time to calculate, so we
        //    should enable/disable this with a flag).
        // 3. Commits behind master

        self.print_status(status, out)?;
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
