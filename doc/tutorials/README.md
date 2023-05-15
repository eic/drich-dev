dRICH Tutorial Series
=====================

Prerequisite Tutorials
----------------------
These are tutorials given to the general ePIC Collaboration. Before following
any dRICH-specific tutorials, it is recommended to at least watch the
**REQUIRED** tutorials. The _optional_ prerequisites are not necessary, but are
helpful to have done in advance of the corresponding dRICH-specific tutorial
(e.g., watch the EICrecon tutorial before dRICH-specific reconstruction
tutorial).

1. Setup of eic-shell and Github (**REQUIRED**): <https://indico.bnl.gov/event/16826/>
2. DD4hep geometry basics (**REQUIRED**): <https://indico.bnl.gov/event/16828/>
3. Simulation with DD4hep (_optional_): <https://indico.bnl.gov/event/16830/>
4. Reconstruction with EICrecon (_optional_): <https://indico.bnl.gov/event/16833/>

Link to all common ePIC tutorials: <https://indico.bnl.gov/category/443/>

dRICH Tutorials
---------------
These tutorials are organized in weekly sessions, first focusing on running the
code with examples, then reading through the corresponding code and providing a
tour of the implementation details.

1. `drich-dev` Setup and Running Simulations
  - clone repositories
    - `check_branches.sh` and `check_status.sh`
    - overriding container builds; show `/usr/local/` and `eic-info`
    - `build.sh` and `environ.sh`, local `prefix/`
    - `make`
    - `rebuild_all.sh`
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

2. dRICH ePIC Geometry Code
  - common compact files (materials, definitions, etc.)
  - dRICH compact file
  - dRICH plugin
  - describe that these are installed in `prefix/` (`$DETECTOR_PATH/`)

3. `drich-dev` Running Reconstruction and Benchmarks
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
  - Benchmarks
    - Checking the output

4. Reconstruction Code Part I
  - Digitization
  - Track Propagation
  - Start IRT

5. Reconstruction Code Part II
  - Continue IRT
  - Merging
  - Linking

6. Special optional tutorial: geometry parameter scanning (brute force optimizer approach)
  - ruby and gems installation
  - describe how to run our generalized parameter scanner
