///////////////////////////////////////////////////////////////////////////////
// NAME:            lib.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Libvremote interface. This library is used to interface
//                  with remote endpoints.
//
// CREATED:         10/01/2021
//
// LAST EDITED:     10/01/2021
////

mod file_remote;

pub struct FileRemote {}

pub struct FileRemoteSpec {
    path: String,
}

pub enum RemoteSpec {
    File(FileRemoteSpec),
}

pub fn remote_type(url: String) -> Result<RemoteSpec, ()> {
    // If it starts with '/', Assume it's an absolute path (don't attempt to
    // parse Windows drive letters, for now)
    if url.starts_with("/") {
        return Ok(RemoteSpec::File(FileRemoteSpec { path: url }));
    }

    // Attempt to obtain a scheme
    let (scheme, path) = match url.find(':') {
        Some(i) => url.split_at(i),
        None => return Err(()),
    };

    let path = path[3..].to_string();
    if scheme == "file" {
        return Ok(RemoteSpec::File(FileRemoteSpec { path }));
    }

    return Err(());
}

///////////////////////////////////////////////////////////////////////////////
