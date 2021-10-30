///////////////////////////////////////////////////////////////////////////////
// NAME:            deployment_policy.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Information about the process of deployment
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

use std::convert::TryFrom;
use std::fmt;
use serde::{Serialize, Deserialize};
use libvruntime::error::VariantError;

#[derive(Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub enum DeploymentPolicy {
    Overwrite,
    NoOverwrite,
}

impl TryFrom<&str> for DeploymentPolicy {
    type Error = VariantError;
    fn try_from(value: &str) -> Result<Self, Self::Error> {
        match &value.to_lowercase().as_str() {
            &"overwrite" => Ok(DeploymentPolicy::Overwrite),
            &"donotoverwrite" => Ok(DeploymentPolicy::NoOverwrite),
            &_ => Err(VariantError::new(value)),
        }
    }
}

impl fmt::Display for DeploymentPolicy {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        fmt::Debug::fmt(self, f)
    }
}

///////////////////////////////////////////////////////////////////////////////
