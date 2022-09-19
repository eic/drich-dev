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
build.sh athena clean
build.sh juggler clean
```

For reconstruction, you may need to create an IRT auxfile, using
```bash
deprecated/create_irt_auxfile_athena.py
```

Revert to EPIC
==============
To switch your environment back to EPIC:
```bash
source environ.sh
build.sh epic clean
build.sh juggler clean
```
