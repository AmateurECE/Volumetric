
use crate::lock::Lock;
use crate::object_store::ObjectStore;

pub struct Stage {
    lock: Lock,
    object_store: ObjectStore,
}

impl Stage {
    pub fn new() -> Stage { Stage {} }
}
