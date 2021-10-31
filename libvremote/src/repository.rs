///////////////////////////////////////////////////////////////////////////////
// NAME:            repository.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic to encapsulate repositories
//
// CREATED:         10/28/2021
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

use std::path::{Path, PathBuf};

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
const DATA_DIR: &'static str        = ".volumetric";
const LOCK_FILE: &'static str       = "lock";
const SETTINGS_FILE: &'static str   = "settings";
const HISTORY_FILE: &'static str    = "history";
const OBJECTS_DIR: &'static str     = "objects";
const CHANGES_DIR: &'static str     = "changes";

const STAGE_DIR: &'static str       = "staging";
const TMP_DIR: &'static str         = "tmp";

pub struct StageStructure {
    stage: PathBuf,
    tmp: PathBuf,
    lock: PathBuf,
    objects: PathBuf,
}

impl<'a> StageStructure {
    pub fn get_stage(&'a self) -> &'a Path { &self.stage }
    pub fn get_tmp(&'a self) -> &'a Path { &self.tmp }
    pub fn get_lock(&'a self) -> &'a Path { &self.lock }
    pub fn get_objects(&'a self) -> &'a Path { &self.objects }
}

impl Default for StageStructure {
    fn default() -> StageStructure {
        let stage = PathBuf::from(DATA_DIR).join(STAGE_DIR);
        StageStructure {
            tmp: stage.to_owned().join(TMP_DIR),
            lock: stage.to_owned().join(LOCK_FILE),
            objects: stage.to_owned().join(OBJECTS_DIR),
            stage,
        }
    }
}

pub struct RepositoryStructure {
    volumetric: PathBuf,
    data: PathBuf,
    lock: PathBuf,
    settings: PathBuf,
    history: PathBuf,
    objects: PathBuf,
    changes: PathBuf,
    stage: StageStructure,
}

impl<'a> RepositoryStructure {
    pub fn get_volumetric(&'a self) -> &'a Path { &self.volumetric }
    pub fn get_data(&'a self) -> &'a Path { &self.data }
    pub fn get_lock(&'a self) -> &'a Path { &self.lock }
    pub fn get_settings(&'a self) -> &'a Path { &self.settings }
    pub fn get_history(&'a self) -> &'a Path { &self.history }
    pub fn get_objects(&'a self) -> &'a Path { &self.objects }
    pub fn get_changes(&'a self) -> &'a Path { &self.changes }
    pub fn get_stage(&'a self) -> &'a StageStructure { &self.stage }
}

impl Default for RepositoryStructure {
    fn default() -> RepositoryStructure {
        let data = PathBuf::from(DATA_DIR);
        RepositoryStructure {
            volumetric: PathBuf::from(VOLUMETRIC_FILE),
            lock: data.to_owned().join(LOCK_FILE),
            settings: data.to_owned().join(SETTINGS_FILE),
            history: data.to_owned().join(HISTORY_FILE),
            objects: data.to_owned().join(OBJECTS_DIR),
            changes: data.to_owned().join(CHANGES_DIR),
            stage: StageStructure::default(),
            data,
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
