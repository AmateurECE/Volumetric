///////////////////////////////////////////////////////////////////////////////
// NAME:            lib.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Libvremote interface. This library is used to interface
//                  with remote endpoints.
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/04/2021
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

extern crate libvruntime;

mod volumetric_remote;
mod file_remote;

// More or less: Maj.Min.Patch
pub const REPOSITORY_VERSION: &'static str = "0.1.0";
pub const DATA_DIR: &'static str = ".volumetric";

// This trait guarantees a consistent interface for remote endpoints.
pub trait RemoteImpl {
    fn get_file(&mut self, name: &str) -> io::Result<Box<dyn io::Read>>;

    fn put_file(&mut self, name: &str, buffer: &[u8]) -> io::Result<usize>;

    fn create_dir(&mut self, name: &str) -> Result<(), io::Error>;
}

pub use volumetric_remote::VolumetricRemote;
pub use file_remote::{FileRemote, FileRemoteSpec};

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
        None => return Ok(RemoteSpec::File(FileRemoteSpec { path: url })),
    };

    let path = path[3..].to_string();
    if scheme == "file" {
        return Ok(RemoteSpec::File(FileRemoteSpec { path }));
    }

    return Err(());
}

///////////////////////////////////////////////////////////////////////////////
