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
// LAST EDITED:     10/10/2021
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
use std::iter::Iterator;

mod command;
mod file_remote;
mod hash;
mod volume;

pub use command::init::Init;
pub use command::add::Add;
pub use command::status::Status;
pub use command::commit::Commit;
pub use command::generate::Generate;
pub use file_remote::{FileRemote, FileRemoteSpec};
pub use command::SettingsFile;

// More or less: Maj.Min.Patch
pub const REPOSITORY_VERSION: &'static str = "0.1.0";

// This trait guarantees a consistent interface for remote endpoints.
pub trait RemoteImpl {
    fn get_file<P: AsRef<Path>>(&mut self, name: P) ->
        io::Result<Box<dyn io::Read>>;

    fn put_file<P: AsRef<Path>>(&mut self, name: P, buffer: &[u8]) ->
        io::Result<usize>;

    fn create_dir<P: AsRef<Path>>(&mut self, name: P) ->
        io::Result<()>;

    fn rename<P: AsRef<Path>, Q: AsRef<Path>>(&mut self, src: P, dest: Q) ->
        io::Result<()>;

    fn copy<P: AsRef<Path>, Q: AsRef<Path>>(&mut self, src: P, dest: Q) ->
        io::Result<()>;

    fn get_path<P: AsRef<Path>>(&self, path: P) -> PathBuf;

    fn read_dir<P: AsRef<Path>>(&self, path: P) ->
        io::Result<Box<dyn Iterator<Item = PathBuf>>>;
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
