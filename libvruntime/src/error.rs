///////////////////////////////////////////////////////////////////////////////
// NAME:            error.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Error type that occurs as a result of attempting to parse
//                  an unknown variant of an enum
//
// CREATED:         10/18/2021
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

use std::error::Error;
use std::fmt;

#[derive(Debug, Clone)]
pub struct VariantError {
    variant: String,
}

impl VariantError {
    pub fn new(value: &str) -> VariantError {
        VariantError { variant: value.to_owned() }
    }
}

impl Error for VariantError {
    fn source(&self) -> Option<&(dyn Error + 'static)> { None }
}

impl fmt::Display for VariantError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "invalid variant: {}", &self.variant)
    }
}

///////////////////////////////////////////////////////////////////////////////
