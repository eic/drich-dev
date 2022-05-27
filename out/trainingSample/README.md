# Training Sample

Sample of dRICH data for GNN training; 
To generate, run `scripts/makeTrainingData.rb`

### Contents:

Several files are produced, per sample; each sample has a unique name in the file
names. The largest ROOT file of each sample contains the tree `events`, which
has branches `MCParticles` for generated particles and `DRICHHits` for the
dRICH information.

Samples:

- `single.*`:
  - particle gun aimed at a fixed momentum
  - for a specific particle and energy, given in file name

- `spray.*`:
  - particle gun aimed at several fixed momenta, distributed throughout the
    polar and azimuthal acceptance of a single dRICH sector
  - fired the same number of particles per fixed momentum
  - for a specific particle and energy, given in file name

- `pythia*`:
  - result from running a Hepmc file from Pythia8
  - you will find hits in all 6 of the dRICH sectors
  - settings such as beam energy and Q2 minimum are given in the file name
