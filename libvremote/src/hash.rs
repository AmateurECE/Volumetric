///////////////////////////////////////////////////////////////////////////////
// NAME:            hash.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Utilities for computing unique hashes.
//
// CREATED:         10/09/2021
//
// LAST EDITED:     10/09/2021
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
use std::io::BufRead;
use std::path::Path;
use std::process;

pub fn sha256sum<P: AsRef<Path>>(path: P) -> io::Result<String> {
    let output = process::Command::new("sha256sum")
        .args([path.as_ref().to_str().unwrap()])
        .output()
        .expect("Error running sha256sum");
    let mut shasum = String::new();
    io::BufReader::new(io::Cursor::new(output.stdout))
        .read_line(&mut shasum)?;
    let shasum = shasum.split_whitespace().nth(0)
        .expect("Improperly formatted output from sha256sum");
    Ok(shasum.to_string())
}

///////////////////////////////////////////////////////////////////////////////
