
use std::io;

pub trait Persistent {
    fn load(target: &dyn io::Read) -> io::Result<Self> where Self: Sized;
    fn store(target: &dyn io::Write) -> io::Result<()>;
}
