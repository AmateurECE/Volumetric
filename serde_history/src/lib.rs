///////////////////////////////////////////////////////////////////////////////
// NAME:            lib.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for the library
//
// CREATED:         10/31/2021
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

use std::error;
use std::fmt;
use std::io;
use std::result;
use serde::{ser, de};

#[derive(Debug, Clone)]
pub struct Error {
    message: String,
}

impl error::Error for crate::Error {
    fn source(&self) -> Option<&(dyn error::Error + 'static)> { None }
}

impl fmt::Display for crate::Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "serialization error (serde_history): {}", &self.message)
    }
}

pub type Result<T> = result::Result<T, crate::Error>;

pub fn from_reader<R, T>(reader: R) -> crate::Result<T>
where
    R: io::Read,
    T: de::DeserializeOwned,
{
    unimplemented!()
}

pub fn to_writer<W, T: ?Sized>(writer: W, value: &T) -> crate::Result<()>
where
    W: io::Write,
    T: ser::Serialize,
{
    unimplemented!()
}

///////////////////////////////////////////////////////////////////////////////
