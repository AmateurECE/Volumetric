///////////////////////////////////////////////////////////////////////////////
// NAME:            file_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     FileRemote implementation
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/03/2021
////

use std::io;
use std::io::Write;
use std::io::ErrorKind;
use std::fs;

use crate::{DATA_DIR, RemoteImpl};

// Remote repository that exists on a currently mounted filesystem.
pub struct FileRemote {
    spec: FileRemoteSpec,
    data_dir: String,
}

// Spec for the FileRemote
pub struct FileRemoteSpec {
    pub path: String,
}

impl FileRemote {
    pub fn new(spec: FileRemoteSpec) -> io::Result<FileRemote> {
        let data_dir = spec.path.to_owned() + "/" + DATA_DIR;
        let mut remote = FileRemote {
            spec, data_dir,
        };
        remote.create_dir("")?; // Pass empty string to ensure we create repo
        Ok(remote)
    }
}

impl RemoteImpl for FileRemote {
    fn get_file(&mut self, _name: &str) -> io::Result<Box<dyn io::Read>> {
        unimplemented!()
    }

    fn put_file(&mut self, name: &str, buffer: &[u8]) -> io::Result<usize> {
        let name = self.data_dir.clone() + "/" + name;
        let mut writer = fs::File::create(&name)?;
        writer.write(&buffer)?;

        Ok(buffer.len())
    }

    fn create_dir(&mut self, name: &str) -> io::Result<()> {
        let name = self.spec.path.to_owned() + "/" + DATA_DIR
            + "/" + name;
        match fs::create_dir_all(&name) {
            Ok(()) => Ok(()),
            Err(e) => match e.kind() {
                // Doesn't matter if it already exists.
                ErrorKind::AlreadyExists => Ok(()),
                _ => Err(e),
            },
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
