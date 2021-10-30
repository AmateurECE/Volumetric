
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

    pub fn add_volume(&mut self, volume: Volume) -> io::Result<()> {
        // let staging_lock = path::PathBuf::from(STAGING_DIR).join("lock");
        // let mut volumes: HashMap<String, Volume>
        //     = match self.transport.get_file(&staging_lock) {
        //         Ok(file) => serde_yaml::from_reader(file).unwrap(),
        //         // If the file does not exist...
        //         Err(e) => match e.kind() {
        //             io::ErrorKind::NotFound => {
        //                 // Attempt to copy it from DATA_DIR
        //                 self.transport.copy(&LOCK_FILE, &staging_lock)?;
        //                 serde_yaml::from_reader(
        //                     self.transport.get_file(&staging_lock)?).unwrap()
        //             },
        //             _ => return Err(e),
        //         },
        //     };

        // volumes.insert(volume.get_name().to_owned(), volume);
        // let volumes = serde_yaml::to_string(&volumes).unwrap();
        // self.transport.put_file(&staging_lock, &volumes.as_bytes())?;
        // Ok(())
        unimplemented!()
    }
}

impl Persistent for Lock {
    fn load(target: &mut dyn io::Read) -> io::Result<Self> {
        Ok(serde_yaml::from_reader::<&mut dyn io::Read, Lock>(target)
           .ok_or(io::Error::new(
               io::ErrorKind::Invalid, "Serialization error")))
    }

    fn store(&self, target: &mut dyn io::Write) -> io::Result<()> {
        let contents = serde_yaml::to_string(&self).ok_or();
        target.write(&contents.as_bytes())?;
        Ok(())
    }
}
