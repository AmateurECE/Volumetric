///////////////////////////////////////////////////////////////////////////////
// NAME:            configuration.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Configuration
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

use std::collections::HashMap;
use std::default::Default;
use std::error::Error;
use std::fs::File;
use std::io::{BufReader, BufWriter};

use serde::{Serialize, Deserialize};

#[derive(Serialize, Deserialize)]
pub enum VolumeConfig {
    External {
        url: String,
        revision: String,
    },
}

#[derive(Serialize, Deserialize)]
pub struct Configuration {
    pub version: u8,
    pub volumes: HashMap<String, VolumeConfig>,
}

impl Default for Configuration {
    fn default() -> Configuration {
        Configuration { version: 1, volumes: HashMap::new() }
    }
}

pub fn load(file: &str) -> Result<Configuration, Box<dyn Error>> {
    let reader = BufReader::new(File::open(file)?);
    Ok(serde_yaml::from_reader::<BufReader<File>, Configuration>(reader)?)
}

pub fn store(file: &str, configuration: &Configuration) ->
    Result<(), Box<dyn Error>>
{
    let writer = BufWriter::new(File::create(file)?);
    Ok(serde_yaml::to_writer::<BufWriter<File>, Configuration>(
        writer, configuration)?)
}

///////////////////////////////////////////////////////////////////////////////
