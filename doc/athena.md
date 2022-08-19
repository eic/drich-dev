Download ATHENA
===============
Clone the `athena` repo, and checkout the `144-irt-geometry` development branch:
```bash
git clone --branch=144-irt-geometry git@eicweb.phy.anl.gov:EIC/detectors/athena.git
```

Switch to ATHENA
================
Source environment:
```bash
source environ.sh                     # EPIC environment
source deprecated/environ_athena.sh   # overrides for ATHENA
```

Build the ATHENA geometry, then re-build Juggler:
```bash
deprecated/build_athena.sh clean
build_juggler.sh clean
```

Revert to EPIC
==============
To switch your environment back to EPIC:
```bash
source environ.sh
build_epic.sh clean
build_juggler.sh clean
```
