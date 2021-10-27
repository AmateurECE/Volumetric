///////////////////////////////////////////////////////////////////////////////
// NAME:            volume.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic encapsulating operations on volume images
//
// CREATED:         10/26/2021
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

use serde::{Serialize, Deserialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Scheme {
    Managed,
    External,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Volume {
    name: String,
    hash: String,
    scheme: Scheme,
}

impl Volume {
    fn new(name: &str, scheme: Scheme) -> Volume {
        Volume {
            name: name.to_owned(),
            hash: "/dev/null".to_string(),
            scheme,
        }
    }

    pub fn managed(name: &str) -> Volume {
        Volume::new(name, Scheme::Managed)
    }

    pub fn external(name: &str) -> Volume {
        Volume::new(name, Scheme::External)
    }

    pub fn get_name(&self) -> &str { &self.name }
    pub fn get_hash(&self) -> &str { &self.hash }
}

///////////////////////////////////////////////////////////////////////////////
