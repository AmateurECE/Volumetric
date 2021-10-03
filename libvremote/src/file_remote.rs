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

use crate::{RemoteImpl, FileRemote, FileRemoteSpec};

impl FileRemote {
    pub fn new(spec: FileRemoteSpec) -> FileRemote {
        FileRemote { spec }
    }
}

impl RemoteImpl for FileRemote {
    fn get_file(filename: &str)
                -> Result<io::BufReader<Box<dyn io::Read>>, io::Error> {
        unimplemented!()
    }

    fn put_file(buffer: io::BufReader<Box<dyn io::Read>>)
                -> Result<usize, io::Error> {
        unimplemented!()
    }
}

impl FileRemoteSpec {
    pub fn get_path(&self) -> &str { self.path.as_str() }
}

///////////////////////////////////////////////////////////////////////////////
