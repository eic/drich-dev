Setup and Running Simulations
=============================

This is the first tutorial of the [dRICH Tutorial Series](README.md), which will cover setup and running of dRICH simulations.

## Prerequisite Common Training
1. Setup of eic-shell and Github: <https://indico.bnl.gov/event/16826/>
1. DD4hep geometry basics: <https://indico.bnl.gov/event/16828/>

## Clone Repositories
First, clone the necessary repositories from [Github](https://github.com) and from [EICweb](https://eicweb.phy.anl.gov/EIC). If you are a member of the [EIC organization on Github](https://github.com/eic) and a member of the [EPIC Devs Team](https://github.com/orgs/eic/teams/epic-devs), you may clone Github repositories with `SSH`, otherwise you must clone with `HTTPS`. Direct contributions to Github repositories requires `SSH` access, otherwise you must fork repositories. The story is the same for [EICweb](https://eicweb.phy.anl.gov/EIC): use `SSH` if you are a member, otherwise use `HTTPS`.

In today's tutorial we will only be working with the following repositories:
- `drich-dev`
- `epic`

These are the only ones you need to clone for today. Future tutorials will use other repositories, so after today's tutorial, it is recommended to clone those as well. Here are the clone commands:

- `drich-dev`
  - `SSH`:
    ```bash
    git clone git@github.com:eic/drich-dev.git
    ```
  - `HTTPS`:
    ```bash
    git clone https://github.com/eic/drich-dev.git
    ```

Once you have a clone of `drich-dev`, you must `cd` into it, prior to cloning the other repositories:
```bash
cd drich-dev
```
Now proceed by cloning the other repositories (today we will only need `epic`):

- Github Repositories:
  - `SSH`:
    ```bash
    git clone git@github.com:eic/epic.git
    git clone git@github.com:eic/irt.git
    git clone git@github.com:eic/EDM4eic.git
    git clone git@github.com:eic/EICrecon.git
    ```
  - `HTTPS`:
    ```bash
    git clone https://github.com/eic/epic.git
    git clone https://github.com/eic/irt.git
    git clone https://github.com/eic/EDM4eic.git
    git clone https://github.com/eic/EICrecon.git
    ```
- EICweb Repositories:
  - `SSH`:
    ```bash
    git clone git@eicweb.phy.anl.gov:EIC/benchmarks/reconstruction_benchmarks.git
    ```
  - `HTTPS`:
    ```bash
    git clone https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks.git
    ```

During development, you may make changes to multiple different repositories, and may be on different branches on each. To help keep track, there are two scripts:
```bash
./check_branches.sh   # list the current branch on every repository
./check_status.sh     # run 'git status' for each repository
```

## Building Repositories
The `eic-shell` image contains all of the necessary software to run ePIC simulations and reconstruction, including builds of the repositories that you cloned above. We make our own clones here in `drich-dev`, so that you may override the `eic-shell` image builds and make changes.

You can find which versions of software are installed in `eic-shell` by running:
```bash
eic-info
```
You may also find the installations themselves at `/usr/local`, which is useful if you want to check header files or libraries.

Let's now build the local repositories, which will override the `eic-shell` builds. From here on, you must be in an `eic-shell` container shell.

First, set the environment variables:
```bash
source environ.sh
```

Practically all repositories use `cmake`, so you may proceed using the typical `cmake` commands. For convenience, `drich-dev` provides a `cmake` wrapper `build.sh` to build any local repository. The syntax is:
```bash
build.sh <REPOSITORY>        # build a repository named <REPOSITORY>
build.sh <REPOSITORY> clean  # remove the cmake buildsystem, and then build
```
By default, the installation prefix (`CMAKE_INSTALL_PREFIX`) will be `$EIC_SHELL_PREFIX`. Running `source environ.sh` has modified your default `$EIC_SHELL_PREFIX` to a local directory: `./prefix`; this directory is where all the software that `build.sh` builds will be installed.

Today, we only need to build `epic`:
```bash
build.sh epic
```
It's not yet necessary, but you may build the other repositories by running:
```bash
build.sh irt
build.sh EDM4eic
build.sh EICrecon
build.sh reconstruction_benchmarks
```
Important: if you choose to build all of the repositories, you need to be sure to build them in the correct order of dependence. You can build all of the repositories in this order by running:
```bash
rebuild_all.sh        # build all the repositories (that you have cloned) in the recommended order
rebuild_all.sh clean  # remove the cmake buildsystems, and then build
```

Notice that the build scripts allow for a `clean` option, which is useful for starting over from a clean state. In case you want to clean everything and start over, simply remove the `./prefix` directory, then run `rebuild_all.sh clean`.

Finally, build the local code in `drich-dev`, which uses a `Makefile`:
```bash
make        # build drich-dev code
make clean  # remove drich-dev build (run 'make' again afterward to rebuild)
```

## Exploring the Geometry

- geometry scripts
  - `geometry.sh`
  - `jsroot`
  - `overlap_check.sh`
  - `search_compact_params.sh`
- simulation 
  - `simulate.py`
  - `draw_hits`
  - `event_display`
  - `simulate.py` for optical studies
- advice
  - some advice on watching github repos
  - troubleshooting: `rm -rf prefix/`, `eic-shell --upgrade`

