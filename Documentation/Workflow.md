# Volumetric Workflow

- [x] `init`: Initialize a repository in a directory.
- [x] `add`: Track changes to a volume in the OCI runtime.
- [ ] `status`: Check for changes in existing containers.
- [ ] `commit`: Snapshot current volume state to the local repository.
- [ ] `generate`: Generate `volumetric.yaml` from a repository.
- [ ] `push`: Push refs to a remote repository.
- [ ] `checkout`: Checkout a local copy of a repository (uses only
      volumetric.yaml)
- [ ] `deploy`: Deploy a snapshot to the OCI runtime.
- [ ] `revert`: Discard local changes to a volume set.
- [ ] `rollback`: Roll a snapshot back to a previous commit and deploy its
      volume set to the OCI runtime.
- [ ] `list`: List objects in the local checkout
- [ ] `prune`: Remove objects from the local checkout.
- [ ] `rm`: Remove a volume from the repository.

# Terminology

* Checkout: Local copy of a remote repository.
* Snapshot: All the metadata necessary to recreate a volume set at a particular
  point in time.
* Volume set: Set of persistent volumes managed by a checkout.
* Deployment: Synonymous with Volume set.
