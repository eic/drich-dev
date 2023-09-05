Tutorial 5: Reconstruction Code Part II
=======================================

- [TUTORIAL RECORDING (mirror 1)](https://drive.google.com/file/d/1BJfRvJeB21Y0PKkd-5Qw1JU32bt7Hboj/view?usp=sharing)
- [Return to Tutorial Landing Page](README.md)

## Introduction

Finally, let's go through the reconstruction code. Refer to [tutorial 4](4-reconstruction-code-part-1.md) for general guidance on the reconstruction code; this tutorial 5 will provide an overview of how the primary algorithms work.

## Navigating EICrecon

Each algorithm "`AlgorithmName`" typically includes 5 source code files:

1. Algorithm class header:         `src/algorithms/___/AlgorithmName.h`
2. Algorithm class implementation: `src/algorithms/___/AlgorithmName.cc`
3. Algorithm configuration:        `src/algorithms/___/AlgorithmNameConfig.h`
4. Factory header:                 `src/global/___/AlgorithmName_factory.h`
5. Factory implementation:         `src/global/___/AlgorithmName_factory.cc`

Recall that the factory runs the algorithm and contains the EICrecon-dependent code, whereas the algorithm is independent of EICrecon.

Note that in some cases the algorithm name and factory name differ; typically this is because the algorithm is more widely used by other subsystems, but we needed a dRICH-specific factory.

Refer to [tutorial 4](4-reconstruction-code-part-1.md) or [dRICH documentation](https://github.com/eic/EICrecon/blob/main/src/detectors/DRICH/README.md) for the list of algorithms and factories specific for the dRICH.

## Tour of the Code

The remainder of this tutorial is a presentation and discussion of the algorithms, how they work, and what can be found where. See the tutorial recording.

### RICH geometry service
`richgeo`:
- aims to be the _only_ place where EICrecon algorithms can know which subsystem they are operating on
- provides conversions of the DD4hep geometry to:
  - ACTS surfaces, for track propagation
  - Optical surfaces for IRT
- also provides general features, such as:
  - enumeration of the radiators
  - readout geometry and visitor methods

### Algorithms
- Digitization
- Track propagation
- IRT: `IrtCherenkovParticleID` and the underlying standalone `irt` code
- Particle ID merging
- Particle ID linking

### Benchmarks
- Simulated hits
- Raw hits
- Cherenkov Particle ID
- Reconstructed Particles
