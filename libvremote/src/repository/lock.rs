
use std::collections::HashMap;
use std::io;
use serde::{Serialize, Deserialize};
use serde_yaml;

use crate::volume::Volume;
use crate::persistence::Persistent;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Lock {
    volumes: HashMap<String, Volume>,
}

impl Lock {
    pub fn add_volume(&mut self, volume: Volume) -> io::Result<()> {
        self.volumes.insert(volume.get_name().to_owned(), volume);
        unimplemented!()
    }
}

impl Default for Lock {
    fn default() -> Lock {
        Lock { volumes: HashMap::new() }
    }
}

impl Persistent for Lock {
    fn load(target: &mut dyn io::Read) -> io::Result<Self> {
        serde_yaml::from_reader::<&mut dyn io::Read, Lock>(target)
            .map_err(|_| io::Error::new(
                io::ErrorKind::InvalidInput, "Deserialization error"))
    }

    fn store(&self, target: &mut dyn io::Write) -> io::Result<()> {
        serde_yaml::to_writer::<&mut dyn io::Write, Lock>(target, &self)
            .map_err(|_| io::Error::new(
                io::ErrorKind::InvalidInput, "Serialization error"))
    }
}
