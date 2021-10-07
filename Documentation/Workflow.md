# Volumetric Workflow

1.  `init`: Initialize a repository in a directory.
2.  `add`: Track changes to a volume in the OCI runtime.
3.  `status`: Check for changes in existing containers.
4.  `commit`: Snapshot current volume state to the local repository.
5.  `push`: Push refs to a remote repository.
6.  `checkout`: Checkout a local copy of a repository (uses only
    volumetric.yaml)
7.  `deploy`: Deploy a snapshot to the OCI runtime.
8.  `revert`: Discard local changes to a volume set.
9.  `rollback`: Roll a snapshot back to a previous commit and deploy its volume
    set to the OCI runtime.
10.  `list`: List objects in the local checkout
11. `prune`: Remove objects from the local checkout.
12. `rm`: Remove a volume from the repository.

# Terminology

* Checkout: Local copy of a remote repository.
* Snapshot: All the metadata necessary to recreate a volume set at a particular
  point in time.
* Volume set: Set of persistent volumes managed by a checkout.
* Deployment: Synonymous with Volume set.
