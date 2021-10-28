
use std::collections::HashMap;
use std::error::Error;
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
    pub fn new() -> Lock {
        Lock { volumes: HashMap::new() }
    }
}

impl Persistent for Lock {
    fn load(target: &mut dyn io::Read) -> Result<Self, Box<dyn Error>> {
        Ok(serde_yaml::from_reader::<&mut dyn io::Read, Lock>(target)?)
    }

    fn store(&self, target: &mut dyn io::Write) -> Result<(), Box<dyn Error>> {
        let contents = serde_yaml::to_string(&self)?;
        target.write(&contents.as_bytes())?;
        Ok(())
    }
}
