
use std::collections::HashMap;
use serde::{Serialize, Deserialize};

use crate::volume::Volume;
use crate::persistence::Persistent;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Lock {
    volumes: HashMap<String, Volume>,
}

impl Lock {
    pub fn new(filename: &str) -> Lock {
        Lock { filename: filename.to_string() }
    }
}

impl Persistent for Lock {
    fn load(target: &dyn io::Read) -> io::Result<Self> {
        unimplemented!()
    }

    fn store(target: &dyn io::Write) -> io::Result<()> {
        unimplemented!()
    }
}
