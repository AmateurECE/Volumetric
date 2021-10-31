///////////////////////////////////////////////////////////////////////////////
// NAME:            remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Abstraction layer for remote endpoints.
//
// CREATED:         10/29/2021
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
use std::iter;
use std::path::{Path, PathBuf};

mod file_remote;
pub use file_remote::{FileRemote, FileRemoteSpec};

// These traits guarantee a consistent interface for remote endpoints.
pub trait ReadRemote<R: io::Read> {
    type Iterator: iter::Iterator;
    fn get_file<P>(&mut self, name: P) -> io::Result<R>
    where R: Sized, P: AsRef<Path>;

    fn read_dir<'a, P>(&'a mut self, name: P) -> io::Result<Self::Iterator>
    where P: AsRef<Path>;

    fn get_path<P>(&self, path: P) -> PathBuf where P: AsRef<Path>;
}

pub trait WriteRemote<R: io::Write + io::Read>: ReadRemote<R> {
    fn upload_file<P>(&mut self, name: P) -> io::Result<R>
    where R: Sized, P: AsRef<Path>;

    fn create_dir<P>(&mut self, name: P) -> io::Result<()>
    where P: AsRef<Path>;

    fn rename<P, Q>(&mut self, src: P, dest: Q) -> io::Result<()>
    where P: AsRef<Path>, Q: AsRef<Path>;

    fn copy<P, Q>(&mut self, src: P, dest: Q) -> io::Result<()>
    where P: AsRef<Path>, Q: AsRef<Path>;
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
