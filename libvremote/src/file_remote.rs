///////////////////////////////////////////////////////////////////////////////
// NAME:            file_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     FileRemote implementation
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/06/2021
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
use std::io::Write;
use std::io::ErrorKind;
use std::fs;

use crate::RemoteImpl;

// Remote repository that exists on a currently mounted filesystem.
pub struct FileRemote {
    spec: FileRemoteSpec,
}

// Spec for the FileRemote
pub struct FileRemoteSpec {
    pub path: String,
}

impl FileRemote {
    pub fn new(spec: FileRemoteSpec) -> io::Result<FileRemote> {
        Ok(FileRemote { spec })
    }
}

impl RemoteImpl for FileRemote {
    fn get_file(&mut self, name: &str) -> io::Result<Box<dyn io::Read>> {
        let name = self.spec.path.clone() + "/" + name;
        Ok(Box::new(fs::File::open(name)?))
    }

    fn put_file(&mut self, name: &str, buffer: &[u8]) -> io::Result<usize> {
        let name = self.spec.path.clone() + "/" + name;
        let mut writer = fs::File::create(&name)?;
        writer.write(&buffer)?;

        Ok(buffer.len())
    }

    fn create_dir(&mut self, name: &str) -> io::Result<()> {
        let name = self.spec.path.to_owned() + "/" + name;
        match fs::create_dir(&name) {
            Ok(()) => Ok(()),
            Err(e) => match e.kind() {
                // Doesn't matter if it already exists.
                ErrorKind::AlreadyExists => Ok(()),
                _ => Err(e),
            },
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
