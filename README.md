# Archive of SAT heritage solver binaries

*This repository archives binaries (i.e., executable files) for all SAT solvers included in the [SAT heritage](https://github.com/sat-heritage/docker-images) initiative.*

**Last updated**: 2025-10-23

## How to Use

The following assumes a CPU with a suitable architecture (usually `linux/amd64`).
It also requires [jq](https://jqlang.org/) to be installed.

```
./run.sh <solver_directory> <dimacs_file>
```

Beware that not all solvers have been tested, so some solvers may require additional dependencies or tweaks.

Generally, this repository is intended to be used from within [torte](https://github.com/ekuiter/torte), which provides full Docker automation for running solvers.

## How to Update

The following command will pull all SAT solvers from Docker and extract their binaries.
All existing solver directories will be overwritten.

```
./setup.sh
```

## Manual Changes

- Removed build files of candy-2019 + mipsat-2013, which are not needed and bloat the repository size

## License

The source code of this project (i.e., `[run|setup].sh`) is released under the [LGPL v3 license](LICENSE.txt).

The distributed solver binaries have been collected or compiled from public sources.
Their usage is subject to each binaries' respective license as specified by the original authors.
Please contact me if you perceive any licensing issues.
