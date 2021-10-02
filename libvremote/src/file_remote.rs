///////////////////////////////////////////////////////////////////////////////
// NAME:            file_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     FileRemote implementation
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/01/2021
////

use crate::FileRemoteSpec;

impl FileRemoteSpec {
    pub fn get_path(&self) -> &str { self.path.as_str() }
}

///////////////////////////////////////////////////////////////////////////////
