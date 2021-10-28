
use crate::repository::lock::Lock;
use crate::repository::object_store::ObjectStore;

pub struct Stage {
    lock: Lock,
    object_store: ObjectStore,
}

impl Stage {
    pub fn new(lock: Lock, object_store: ObjectStore) -> Stage {
        Stage { lock, object_store }
    }

    pub fn get_mut_lock<'a>(&'a mut self) -> &'a mut Lock { &mut self.lock }
}
