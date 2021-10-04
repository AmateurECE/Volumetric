# Structure of Remote Repositories

```
.volumetric/
+-lock
+-settings
+-history
+-objects/
  +-d967ae99784dc10b86a6eb...(SHA-256 sum)
+-changes/
  +-d1b3af83a2fa698a0bc7ed...(SHA-256 sum)
```

# `lock`

A YAML file describing the configuration of a working set of volume images. In
general, each volume is a `dict` containing (at the very least) the name of the
volume in the container runtime and a SHA-256 sum corresponding to one of the
objects in `objects/`.

# `settings`

A YAML file containing settings for the repository. At a minimum, settings
must contain the field `version`, which is used to be able to handle changes
in the settings schema, repository format, and all other characteristics of
the repository.

# `history`

List of entries in `changes/`, ordered chronologically. This file is a table,
the left column of which is the name of an entry in `changes/`, the right
column of which is the symbolic name for the revision.

# `objects/`

The actual volume images. Each entry in this directory is a volume image, which
is a gzipped tar archive (`.tar.gz`) of the volume's contents. The entries are
named by the SHA-256 sum of the gzipped archive's contents.

# `changes/`

Each entry in this directory is a gzipped patch that chronicles the history of
the `lock` file. The name of the file is the SHA-256 sum of its gzipped
contents.
