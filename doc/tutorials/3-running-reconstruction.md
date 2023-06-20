Tutorial 3: Running Reconstruction and Benchmarks
=================================================

- TUTORIAL RECORDING
- [Return to Tutorial Landing Page](README.md)

## Prerequisites
Before attending this tutorial, please build the reconstruction and benchmarks software, since this may take some time; this section describes how to do so.

### Obtaining the Software

At the time of writing this tutorial, dRICH PID is still not fully approved in the `main` branches of the reconstruction and benchmarks software. The current development branch is `irt-algo` for both; `irt-algo` on EICrecon is somewhat unstable, so instead I recommend you to use `irt-algo-stable`, which at times may be a bit behind `irt-algo`.

If you already have clones of `EICrecon` and `reconstruction_benchmarks`, you can switch branches by running:
```bash
pushd EICrecon
git fetch origin
git checkout irt-algo-stable    # or irt-algo, if you want bleeding edge
popd
pushd reconstruction_benchmarks
git fetch origin
git checkout irt-algo    # no irt-algo-stable branch, since this code doesn't change as rapidly
popd
```

Otherwise if you do not yet have clones of `EICrecon` and `reconstruction_benchmarks`, you can clone and checkout the appropriate branch in one command. As in [tutorial 1](1-setup-and-running-simulations.md), use HTTPS or SSH depending on your access credentials:

- `EICrecon` SSH:
```bash
git clone git@github.com:eic/EICrecon.git --branch irt-algo-stable       # or irt-algo, if you want bleeding edge
```
- `EICrecon` HTTPS:
```bash
git clone https://github.com/eic/EICrecon.git --branch irt-algo-stable   # or irt-algo, if you want bleeding edge
```
- `reconstruction_benchmarks` SSH:
```bash
git clone git@eicweb.phy.anl.gov:EIC/benchmarks/reconstruction_benchmarks.git --branch irt-algo
```
- `reconstruction_benchmarks` HTTPS:
```bash
git clone https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks.git --branch irt-algo
```

Verify that you are on the correct set of branches by running `./check_branches.sh`. The output should look something like:
```
                     drich-dev: main  (269bf3c)
                          epic: main  (d14e80b)
                       EDM4eic: NOT INSTALLED
                           irt: NOT INSTALLED
                      EICrecon: irt-algo-stable  (21fd271a)
                       juggler: NOT INSTALLED
     reconstruction_benchmarks: irt-algo  (d1c7885)
```
(the commit hashes, in parentheses, may differ)

### Building

First, if you followed the previous tutorials, your `epic` repository may be in a non-default state. You can check this with either `./check_status.sh`, which runs `git status` on all repositories, or `cd epic` then run `git status`. If you see you have made changes, run `git diff` to show them. Revert your changes, if there are any.

Regardless of whether you made any changes in `epic` or not, it is recommended to rebuild `epic` in case you forgot that you made changes and have a modified build. Run `build.sh epic` (or your preferred `cmake` commands) to rebuild.

Now build the reconstruction and benchmarks code. You can use `build.sh` (see [tutorial 1](1-setup-and-running-simulations.md)) or your preferred `cmake` commands. If using `build.sh`, run the commands below. This will take some time; consider reducing the environment variable `BUILD_NPROC` so that the compilation does not use too many resources.
```bash
build.sh EICrecon
build.sh reconstruction_benchmarks
```
or just run:
```bash
rebuild_all.sh
```

Now you are ready to follow along with the interactive tutorial!


## UNDER CONSTRUCTION
The rest of this tutorial is under construction; here is a rough outline:

- our `mermaid` diagram
  - JANA calls only what it needs
  - Show other `mermaid` diagrams, if needed
- JANA2 objects
  - Collections
  - `EDM4*` data model: our `datatypes` (ref. Thomas's PODIO talk)
  - Factories
  - Algorithms and EICrecon independence (ref. Sylvester's talk)
  - Configurations
  - Plugins
- `recon.rb` and our config files
  - CLI-level config (from our config files) 
  - Expected common-level refactoring to `toml`
  - `DRICH.cc` level config
  - Algorithm level default configs
- `event_display`
  - compare simulation to reconstruction
- Benchmarks
  - Checking the output

