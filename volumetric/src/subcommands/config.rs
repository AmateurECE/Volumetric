///////////////////////////////////////////////////////////////////////////////
// NAME:            config.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     View or mutate the repository settings.
//
// CREATED:         10/29/2021
//
// LAST EDITED:     10/29/2021
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
use clap::ArgMatches;
use libvremote::WriteRemote;

// TODO: Eventually, may want to allow reading config from a Read Remote, but
// not writing (i.e. should allow an overload taking only a ReadRemote).
pub fn config<P, R>(remote: R, matches: ArgMaches) ->
    Result<(), Box<dyn Error>>
where
    P: io::Read + io::Write,
    R: WriteRemote<R>
{
        let stdout = io::stdout();
        let mut stdout = stdout.lock();
        let option = matches.value_of("option");
        if matches.is_present("list") {
            if let Some(option) = option {
                // Print option
                let value = settings.get(&option)?;
                write!(stdout, "{}: {}\n", &option, &value)?;
            } else {
                // Print entire configuration
                settings.iter()
                    .for_each(|(k, v)| write!(stdout, "{}: {}\n", &k, &v)
                              .unwrap());
            }
        } else {
            // Set option to value (or unset)
            settings.set(option.unwrap(), matches.value_of("value"))?;
            // Write settings
            libvremote::write_settings(&mut remote, &settings)?;
        }
}

///////////////////////////////////////////////////////////////////////////////
