///////////////////////////////////////////////////////////////////////////////
// NAME:            lib.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Libvremote interface. This library is used to interface
//                  with remote endpoints.
//
// CREATED:         10/01/2021
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

mod compressor;
mod remote;
mod hash;
mod persistence;
mod repository;
mod volume;

pub use compressor::Compressor;
pub use persistence::Persistent;

pub use remote::ReadRemote;
pub use remote::WriteRemote;
pub use remote::RemoteSpec;
pub use remote::remote_type;
pub use remote::{FileRemote, FileRemoteSpec};

pub use repository::DeploymentPolicy;
pub use repository::Lock;
pub use repository::ObjectStore;
pub use repository::RepositoryStructure;
pub use repository::Settings;
pub use repository::Stage;
pub use repository::StageStructure;

pub use volume::Volume;

// More or less: Maj.Min.Patch
pub const REPOSITORY_VERSION: &'static str = "0.1.0";

///////////////////////////////////////////////////////////////////////////////
