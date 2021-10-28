///////////////////////////////////////////////////////////////////////////////
// NAME:            docker.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Abstraction layer for working with Docker.
//
// CREATED:         10/04/2021
//
// LAST EDITED:     10/12/2021
//
// Copyright 2021, Ethan D. Twardy
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////

use std::error::Error;
use std::io;
use std::io::BufRead;
use std::path;
use std::process;

use crate::OciRuntime;

pub struct Docker {}

impl Docker {
    pub fn new() -> Docker { Docker {} }
}

impl OciRuntime for Docker {
    fn volume_exists(&self, volume: &str) -> Result<bool, Box<dyn Error>> {
        let output = process::Command::new("docker")
            .args(&["volume", "ls"])
            .output()
            .expect("Error running docker volume ls");
        let output = io::BufReader::new(io::Cursor::new(output.stdout));

        // Start from the second, first line is header.
        for line in output.lines().skip(1) {
            let line = line?;
            if let Some(name) = line.split_whitespace().nth(1) {
                if volume == name {
                    return Ok(true)
                }
            }
        }

        Ok(false)
    }

    fn get_volume_host_path(&self, volume: &str) ->
        Result<path::PathBuf, Box<dyn Error>>
    {
        let output = process::Command::new("podman")
            .args(&["inspect", "-f", "{{.Mountpoint}}", &volume])
            .output()
            .expect("Error running podman inspect");
        let mut mount_point = String::new();
        io::BufReader::new(io::Cursor::new(output.stdout))
            .read_line(&mut mount_point)?;
        Ok(path::PathBuf::from(mount_point.trim()))
    }

    fn remove_volume(&self, volume: &str) -> Result<(), Box<dyn Error>> {
        let status = process::Command::new("podman")
            .args(&["volume", "rm", &volume])
            .status()
            .expect("Error running podman volume");
        if !status.success() {
            return Err(Box::new(io::Error::last_os_error()));
        }

        Ok(())
    }

    fn create_volume(&self, volume: &str) -> Result<(), Box<dyn Error>> {
        let status = process::Command::new("podman")
            .args(&["volume", "create", &volume])
            .status()
            .expect("Error running podman volume");
        if !status.success() {
            return Err(Box::new(io::Error::last_os_error()));
        }

        Ok(())
    }
}

///////////////////////////////////////////////////////////////////////////////
