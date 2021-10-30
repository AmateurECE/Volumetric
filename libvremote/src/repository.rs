///////////////////////////////////////////////////////////////////////////////
// NAME:            repository.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to encapsulate repositories
//
// CREATED:         10/28/2021
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

mod lock;
mod object_store;
mod settings;
mod stage;

pub use lock::Lock;
pub use object_store::ObjectStore;
pub use settings::Settings;
pub use settings::DeploymentPolicy;
pub use stage::Stage;

const VOLUMETRIC_FILE: &'static str = "volumetric.yaml";
const LOCK_FILE: &'static str       = ".volumetric/lock";
const SETTINGS_FILE: &'static str   = ".volumetric/settings";
const HISTORY_FILE: &'static str    = ".volumetric/history";
const OBJECTS_DIR: &'static str     = ".volumetric/objects";
const CHANGES_DIR: &'static str     = ".volumetric/changes";

const TMP_DIR: &'static str         = ".volumetric/staging/tmp";
const STAGING_LOCK: &'static str    = ".volumetric/staging/lock";
const STAGING_OBJECTS: &'static str = ".volumetric/staging/objects";

pub struct StageStructure {}
impl StageStructure {
    pub fn get_tmp() -> &'static str { &TMP_DIR }
    pub fn get_lock() -> &'static str { &STAGING_LOCK }
    pub fn get_objects() -> &'static str { &STAGING_OBJECTS }
}

pub struct RepositoryStructure {}
impl RepositoryStructure {
    pub fn get_volumetric() -> &'static str { &VOLUMETRIC_FILE }
    pub fn get_lock() -> &'static str { &LOCK_FILE }
    pub fn get_settings() -> &'static str { &SETTINGS_FILE }
    pub fn get_history() -> &'static str { &HISTORY_FILE }
    pub fn get_objects() -> &'static str { &OBJECTS_DIR }
    pub fn get_changes() -> &'static str { &CHANGES_DIR }
    pub fn get_stage() -> StageStructure { return StageStructure {} }
}

///////////////////////////////////////////////////////////////////////////////
