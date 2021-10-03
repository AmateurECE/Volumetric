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
// LAST EDITED:     10/03/2021
////

use std::io;

mod volumetric_remote;
mod file_remote;

// This trait guarantees a consistent interface for remote endpoints.
pub trait RemoteImpl {
    fn get_file(filename: &str)
                -> Result<io::BufReader<Box<dyn io::Read>>, io::Error>;
    fn put_file(buffer: io::BufReader<Box<dyn io::Read>>)
                -> Result<usize, io::Error>;
}

// This struct implements functionality for talking to a remote repository.
pub struct VolumetricRemote<R: RemoteImpl> {
    transport: R,
}

// Remote repository that exists on a currently mounted filesystem.
pub struct FileRemote {
    spec: FileRemoteSpec,
}

// Spec for the FileRemote
pub struct FileRemoteSpec {
    path: String,
}

// Enum to differentiate between Remotes
pub enum RemoteSpec {
    File(FileRemoteSpec),
}

// Parse a Remote URI and return some information about it to create a Remote
// endpoint
// URI Scheme:
//   <scheme>://[user@]<hostname|IP>[:port]/<path>
// Plan to support schemes: ftp, sftp, ssh, nfs(?), and file.
// If ':' is not in the string, attempt to make it a path.
pub fn remote_type(url: String) -> Result<RemoteSpec, ()> {
    // Attempt to obtain a scheme
    let (scheme, path) = match url.find(':') {
        Some(i) => url.split_at(i),

        // If no scheme is found, assume it's a path.
        None => return Ok(RemoteSpec::File(FileRemoteSpec { path: url })),
    };

    let path = path[3..].to_string();
    if scheme == "file" {
        return Ok(RemoteSpec::File(FileRemoteSpec { path }));
    }

    return Err(());
}

///////////////////////////////////////////////////////////////////////////////
