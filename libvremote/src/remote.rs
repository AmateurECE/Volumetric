///////////////////////////////////////////////////////////////////////////////
// NAME:            remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Abstraction layer for remote endpoints.
//
// CREATED:         10/29/2021
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
use std::path::{Path, PathBuf};

// These traits guarantees a consistent interface for remote endpoints.
pub trait ReadRemote<R: io::Read> {
    fn get_file(&mut self, name: &Path) -> io::Result<R> where R: Sized;
    fn read_dir(&mut self, name: &Path) ->
        io::Result<Box<dyn Iterator<Item = PathBuf>>>;
    fn get_path(&self, path: &Path) -> PathBuf;
}

pub trait WriteRemote<R: io::Write + io::Read>: ReadRemote<R> {
    fn upload_file(&mut self, name: &Path) -> io::Result<R> where R: Sized;
    fn create_dir(&mut self, name: &Path) -> io::Result<()>;
    fn rename(&mut self, src: &Path, dest: &Path) -> io::Result<()>;
    fn copy(&mut self, src: &Path, dest: &Path) -> io::Result<()>;
}


// Enum to differentiate between Remotes
pub enum RemoteSpec {
    File(FileRemoteSpec),
}

// Parse a Remote URI and return some information about it to create a Remote
// endpoint
// URI Scheme:
//   <scheme>://[user@]<hostname|IP>[:port]/<path>
// Plan to support schemes: ftp, sftp, ssh, nfs(?), and file.
// If ':' is not in the string, attempt to make it a path.
pub fn remote_type(url: String) -> Result<RemoteSpec, ()> {
    // Attempt to obtain a scheme
    let (scheme, path) = match url.find(':') {
        Some(i) => url.split_at(i),

        // If no scheme is found, assume it's a path.
        None => return Ok(RemoteSpec::File(FileRemoteSpec {
            path: PathBuf::from(url) })),
    };

    let path = path[3..].to_string();
    if scheme == "file" {
        return Ok(RemoteSpec::File(FileRemoteSpec {
            path: PathBuf::from(path) }));
    }

    return Err(());
}

///////////////////////////////////////////////////////////////////////////////
