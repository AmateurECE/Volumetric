///////////////////////////////////////////////////////////////////////////////
// NAME:            commit.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Commits staged changes to the repository.
//
// CREATED:         10/10/2021
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
use std::io::BufRead;
use std::path;

use crate::RemoteImpl;
use crate::hash;
use crate::command::{
    STAGING_DIR, LOCK_FILE, CHANGES_DIR, HISTORY_FILE, OBJECTS_DIR,
};

pub struct Commit<R: RemoteImpl> {
    transport: R,
}

impl<R: RemoteImpl> Commit<R> {
    pub fn new(transport: R) -> Commit<R> {
        Commit { transport }
    }

    // Commit staged changes
    pub fn commit(&mut self) -> io::Result<()> {
        // 1. Move .volumetric/staging/lock to .volumetric/lock
        let staging_lock = path::PathBuf::from(STAGING_DIR).join("lock");
        self.transport.rename(&staging_lock, &LOCK_FILE)?;

        // 2. Hash .volumetric/lock
        let lock_path = self.transport.get_path(&LOCK_FILE);
        let lock_hash = hash::sha256sum(&lock_path)?;

        // 3. Copy .volumetric/lock to .volumetric/changes/<hash>
        let object_file = path::PathBuf::from(&CHANGES_DIR).join(&lock_hash);
        self.transport.copy(&LOCK_FILE, &object_file)?;

        // 4. Append <hash> to .volumetric/history
        let history = io::BufReader::new(
            self.transport.get_file(&HISTORY_FILE)?)
            .lines().map(|l| l.unwrap())
            .collect::<Vec<String>>().join("\n");
        let history = history + &lock_hash + "\n";
        self.transport.put_file(&HISTORY_FILE, &history.as_bytes())?;

        // 5. Move .volumetric/staging/objects/* to .volumetric/objects/
        let staging_objects = path::PathBuf::from(STAGING_DIR).join("objects");
        for staged_object in self.transport.read_dir(&staging_objects)? {
            let object_name = path::PathBuf::from(OBJECTS_DIR)
                .join(staged_object.file_name().unwrap().to_str().unwrap());
            self.transport.rename(staged_object, object_name)?;
        }
        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
