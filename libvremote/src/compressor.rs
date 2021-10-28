///////////////////////////////////////////////////////////////////////////////
// NAME:            compressor.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implements logic around a volume.
//
// CREATED:         10/09/2021
//
// LAST EDITED:     10/26/2021
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
use std::path::Path;
use std::process;

pub struct Compressor {}
impl Compressor {
    pub fn new() -> Compressor { Compressor {} }

    // Stage a snapshot of a volume for commit. Return the hash.
    pub fn stage<P, Q>(&mut self, host_path: P, temp: Q) ->io::Result<()>
    where P: AsRef<Path>, Q: AsRef<Path>,
    {
        let status = process::Command::new("tar")
            .args(&["czvf", temp.as_ref().to_str().unwrap(),
                   "-C", host_path.as_ref().to_str().unwrap(), "."])
            .status()
            .expect("Error running tar");
        if !status.success() {
            return Err(io::Error::from_raw_os_error(
                status.code().unwrap()));
        }

        Ok(())
    }

    pub fn restore<P, Q>(&self, host_path: P, image: Q) -> io::Result<()>
    where P: AsRef<Path>, Q: AsRef<Path>,
    {
        let status = process::Command::new("tar")
            .args(&["xzvf", image.as_ref().to_str().unwrap(),
                   "-C", host_path.as_ref().to_str().unwrap()])
            .status()
            .expect("Error running tar");
        if !status.success() {
            return Err(io::Error::from_raw_os_error(
                status.code().unwrap()));
        }

        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
