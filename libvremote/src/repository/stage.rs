///////////////////////////////////////////////////////////////////////////////
// NAME:            stage.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     The stage is the location where new versions of volumes can
//                  be prepared for commit.
//
// CREATED:         10/30/2021
//
// LAST EDITED:     10/30/2021
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

use crate::repository::lock::Lock;
use crate::repository::object_store::ObjectStore;

pub struct Stage {
    lock: Lock,
    object_store: ObjectStore,
}

impl Stage {
    pub fn new(lock: Lock, object_store: ObjectStore) -> Stage {
        Stage { lock, object_store }
    }

    // pub fn get_mut_lock<'a>(&'a mut self) -> &'a mut Lock { &mut self.lock }
}

///////////////////////////////////////////////////////////////////////////////
