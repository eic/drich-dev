Legacy ATHENA Support
=====================

Clone `athena` repo:
```bash
git clone git@eicweb.phy.anl.gov:EIC/detectors/athena.git
```

Checkout the development branch (or your preferred branch):
```bash
pushd athena
git checkout 144-irt-geometry
popd
```

Source environment. The `deprecated/environ_athena.sh` file will override some
variables for ATHENA; source `environ.sh` again to revert to EPIC
```bash
source environ.sh
source deprecated/environ_athena.sh
```

Build the ATHENA geometry:
```bash
deprecated/build_athena.sh
```
