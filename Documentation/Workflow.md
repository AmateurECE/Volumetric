# Volumetric Workflow

1. `init`: Initialize a repository in a directory.
2. `add`: Track changes to a volume in the OCI runtime.
3. `checkout`: Checkout a local copy of a repository.
4. `deploy`: Deploy a snapshot to the OCI runtime.
5. `status`: Check for changes in existing containers.
6. `commit`: Snapshot current volume state and upload to repository (much more
   like SVN commit than git-commit).
7. `revert`: Discard local changes to a volume set.
8. `rollback`: Roll a snapshot back to a previous commit and deploy its volume
   set to the OCI runtime.

# Terminology

* Checkout: Local copy of a remote repository.
* Snapshot: All the metadata necessary to recreate a volume set at a particular
  point in time.
* Volume set: Set of persistent volumes managed by a checkout.
* Deployment: Synonymous with Volume set.
