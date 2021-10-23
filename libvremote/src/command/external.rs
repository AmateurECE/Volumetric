///////////////////////////////////////////////////////////////////////////////
// NAME:            external.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     External command logic
//
// CREATED:         10/18/2021
//
// LAST EDITED:     10/18/2021
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

use std::error::Error;

use crate::volume::Volume;
use crate::RemoteImpl;
use crate::settings::Settings;

pub struct External<R: RemoteImpl> {
    transport: R,
    settings: Settings,
}

impl<R: RemoteImpl> External<R> {
    pub fn new(transport: R, settings: Settings) -> External<R> {
        External { transport, settings }
    }

    pub fn add(&mut self, volume: Volume, hash: String, uri: String) ->
        Result<(), Box<dyn Error>>
    {
        unimplemented!()
    }
}

///////////////////////////////////////////////////////////////////////////////
