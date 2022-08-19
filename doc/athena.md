Legacy ATHENA Support
=====================

Clone the `athena` repo, and checkout the `144-irt-geometry` development branch:
```bash
git clone --branch=144-irt-geometry git@eicweb.phy.anl.gov:EIC/detectors/athena.git
```

Source environment:
```bash
source environ.sh                     # EPIC environment
source deprecated/environ_athena.sh   # overrides for ATHENA
```

Build the ATHENA geometry, then re-build Juggler:
```bash
deprecated/build_athena.sh
build_juggler.sh clean
```

Reverting to EPIC
=================

To switch your environment back to EPIC:
```bash
source environ.sh
rebuild_all.sh clean
```
