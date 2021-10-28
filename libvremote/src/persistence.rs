
use std::io;
use std::error::Error;

pub trait Persistent {
    fn load(target: &mut dyn io::Read) -> Result<Self, Box<dyn Error>> where Self: Sized;
    fn store(&self, target: &mut dyn io::Write) -> Result<(), Box<dyn Error>>;
}
