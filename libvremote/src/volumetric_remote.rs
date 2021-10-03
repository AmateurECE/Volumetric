///////////////////////////////////////////////////////////////////////////////
// NAME:            volumetric_remote.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Impl for the VolumetricRemote struct.
//
// CREATED:         10/03/2021
//
// LAST EDITED:     10/03/2021
////

use std::error::Error;

use crate::{RemoteImpl, VolumetricRemote};

impl<R: RemoteImpl> VolumetricRemote<R> {
    pub fn new(transport: R) -> VolumetricRemote<R> {
        VolumetricRemote { transport }
    }

    pub fn init(&mut self) -> Result<(), Box<dyn Error>> {
        unimplemented!()
    }
}

///////////////////////////////////////////////////////////////////////////////
