# Active Merge Requests and Issues

## `irt`
- TODO: [rename eRICH to pfRICH](https://eicweb.phy.anl.gov/EIC/irt/-/issues/6)

## `detectors/athena`
### BRANCH `144-irt-geometry` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/331)
- TODO: discuss with SW WG if this approach is okay
- TODO: review this MR carefully; CI will *not work* until `irt` is in upstream
- cloned and rebased from branch `irt-init-v01`
  - validate the rebase was successful:
    - first, use the following command to check what changes `irt-init-v01` was originally introducing; this is useful to know which files are changing; commit `18003ba` is where `irt-init-v01` branched from `master` 
      ```
      git diff 18003ba..origin/irt-init-v01
      ```
    - then, for each file we changed, use `git diff` to check the relevant differences between `irt-init-v01` and `144-irt-geometry`. It is not practical to `git diff` the full branches, since you would pick up a lot of irrelevant changes in other detectors, etc.
      ```
      git diff origin/irt-init-v01:src/DRich_geo.cpp origin/144-irt-geometry:src/DRICH_geo.cpp
      git diff origin/irt-init-v01:src/ERich_geo.cpp origin/144-irt-geometry:src/PFRICH_geo.cpp
      git diff origin/irt-init-v01:compact/drich.xml origin/144-irt-geometry:compact/drich.xml
      git diff origin/irt-init-v01:compact/erich.xml origin/144-irt-geometry:compact/pfrich.xml
      git diff origin/irt-init-v01:compact/optical_materials.xml origin/144-irt-geometry:compact/optical_materials.xml
      git diff origin/irt-init-v01:CMakeLists.txt origin/144-irt-geometry:CMakeLists.txt
      ```
    - I have confirmed that these diffs are only the name change eRICH to pfRICH, along with the introduction of "lores" pfRICH sensor sizes, which are only used in CI visualization
### BRANCH `drich-two-mirrors` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/260) (draft)
### BRANCH `129-update-erich-name` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/328) (approved, not yet merged)

## `eicd`
### BRANCH `irt-data-model` - [MR](https://eicweb.phy.anl.gov/EIC/eicd/-/merge_requests/70)
- cloned and rebased from `irt-init-v01`

## `Project Juggler`

## `benchmarks/reconstruction_benchmarks`

# Miscellaneous Notes:
- how to clone and rebase a branch
  - it is dangerous to rebase a shared branch, thus a reasonable approach is to make a "clone" of a branch, and rebase the clone. The original branch will remain intact.
  - example procedure: clone and rebase `irt-init-v01` on `eicd`
    - check the [commit DAG](https://eicweb.phy.anl.gov/EIC/eicd/-/network/master) to get a sense of the history; if it is kind of messy (e.g., contains merge commits), you may consider an interactive rebase (`rebase -i`) instead of the non-interactive rebase described below. In this example, the history looks clean.
    - at any point, check your local DAG with `git log --decorate --oneline --graph`; it's useful to do this often
    - be sure you are on `master` and are up to date: `git checkout master && git fetch origin && git pull`
    - checkout the branch you want to clone, and update: `git checkout irt-init-v01 && git fetch origin && git pull`
    - make a new branch, called `irt-data-model`: `git checkout -b irt-data-model` (make sure this branch name is not already in use on gitlab)
    - rebase `irt-data-model` to `master`: `git rebase origin/master`
      - in my case, I got conflicts: `git status` reveals `eic_data.yaml` has conflicts, which you can find by opening the file and searching for the word `HEAD`:
        ```
        <<<<<<< HEAD
          ## A point along a track
          eic::TrackPoint:
            Members:
              - eic::VectorXYZ    position        // Position of the trajectory point [mm]
              - eic::CovXYZ       positionError   // Error on the position
              - eic::VectorXYZ    momentum        // 3-momentum at the point [GeV]
              - eic::CovXYZ       momentumError   // Error on the 3-momentum
              - float             time            // Time at this point [ns]
              - float             timeError       // Error on the time at this point
              - float             theta           // polar direction of the track at the surface [rad]
              - float             phi             // azimuthal direction of the track at the surface [rad]
              - eic::CovXY        directionError  // Error on the polar and azimuthal angles
              - float             pathlength      // Pathlength from the origin to this point
              - float             pathlengthError // Error on the pathlenght
         
        =======
          ## A point along a trajectory
          eic::TrajectoryPoint:
            Members:
              - eic::VectorXYZ    position      // Position of the trajectory point [mm]
              - eic::VectorXYZ    p             // 3-momentum at the point [GeV]
              - eic::Direction    direction     // (theta, phi) direction of track at the surface [mrad]
              - float             momentum      // magnitude of 3-momentum [GeV]
              - float             pathlength    // Pathlength from the origin to this point

          ## PID hypothesis from Cherenkov detectors
          eic::CherenkovPdgHypothesis:
            Members:
              - int32_t           pdg               // PDG code
              - int16_t           npe               // Overall p.e. count associated with this hypothesis for a given track
              - float             weight            // The weight associated with this hypothesis (the higher the more probable)

        >>>>>>> Intermediate version with CherenkovParticleID and Trajectory in one file
        ```
        - every conflict is different and takes some thought to resolve. In this case, it seems we want both versions, so I just deleted the lines that start with `<<<<`, `====`, and `>>>>`.
      - `git add eic_data.yaml` marks the conflict resolved
      - `git rebase --continue` to continue with the rebase, and resolve more conflicts as need. In this example, there were no more conflicts.
    - once the rebase no longer complains, check your local DAG, and use the `--all` option to see the entire DAG, to verify you made a clone of the original branch, rebased to the current `origin/master`
    - push the new branch: since it does not exist on gitlab, you must do `git push -u origin irt-data-model`
    - finish by opening a new merge request
