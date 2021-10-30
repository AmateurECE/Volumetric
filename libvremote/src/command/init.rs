///////////////////////////////////////////////////////////////////////////////
// NAME:            init.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Initializes a repository.
//
// CREATED:         10/10/2021
//
// LAST EDITED:     10/28/2021
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

use std::collections::HashMap;
use std::error::Error;
use std::path;

use crate::RemoteImpl;
use crate::volume::Volume;
use crate::repository::Settings;
use crate::repository::Lock;
use crate::command::{
    DATA_DIR, SETTINGS_FILE, HISTORY_FILE, CHANGES_DIR, STAGING_DIR, TMP_DIR,
    OBJECTS_DIR, LOCK_FILE,
};

pub struct Init<R: RemoteImpl> {
    transport: R,
    settings: Settings,
}

impl<R: RemoteImpl> Init<R> {
    pub fn new(transport: R, settings: Settings) -> Init<R> {
        Init { transport, settings }
    }

    // Populate all the initial artifacts
    pub fn init(&mut self) -> Result<(), Box<dyn Error>> {
        self.transport.create_dir(&DATA_DIR)?;
        let settings = self.settings.to_string();
        self.transport.put_file(&SETTINGS_FILE, &settings.as_bytes())?;
        self.transport.put_file(
            &LOCK_FILE, &serde_yaml::to_string(
                &HashMap::<String, Volume>::new())?.as_bytes())?;
        self.transport.put_file(&HISTORY_FILE, "".as_bytes())?;

        self.transport.create_dir(&OBJECTS_DIR)?;
        self.transport.create_dir(&CHANGES_DIR)?;
        self.transport.create_dir(&STAGING_DIR)?;
        self.transport.create_dir(&TMP_DIR)?;

        let staging_objects = path::PathBuf::from(STAGING_DIR).join("objects");
        self.transport.create_dir(&staging_objects.to_str().unwrap())?;

        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
