///////////////////////////////////////////////////////////////////////////////
// NAME:            external.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     External volumes.
//
// CREATED:         01/08/2022
//
// LAST EDITED:     01/08/2022
//
// Copyright 2022, Ethan D. Twardy
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

use crate::volume::Volume;

pub struct ExternalVolume {
    name: String,
    url: String,
    revision: String,
}

impl ExternalVolume {
    pub fn new(name: &str, url: &str, revision: &str) -> ExternalVolume {
        ExternalVolume {
            name: name.to_string(),
            url: url.to_string(),
            revision: revision.to_string(),
        }
    }
}

impl Volume for ExternalVolume { }

///////////////////////////////////////////////////////////////////////////////
