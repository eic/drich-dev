# Active Merge Requests and Issues

## `irt`
- TODO: [rename eRICH to pfRICH](https://eicweb.phy.anl.gov/EIC/irt/-/issues/6)

## `detectors/athena`
- branch `144-irt-geometry` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/331)
  - TODO: discuss with SW WG if this approach is okay
  - TODO: review this MR carefully; CI will *not work* until `irt` is in upstream
  - cloned and rebased from branch `irt-init-v01`
    - validate the rebase was successful:
      - first, use the following command to check what changes `irt-init-v01` was originally introducing; this is useful to know which files are changing; commit `18003ba` is where `irt-init-v01` branched from `master` 
        ```
        git diff 18003ba..origin/irt-init-v01
        ```
      - then, for each file we changed, use `git diff` to check the relevant differences between `irt-init-v01` and `144-irt-geometry`:
        ```
        git diff origin/irt-init-v01:src/DRich_geo.cpp origin/144-irt-geometry:src/DRICH_geo.cpp
        git diff origin/irt-init-v01:src/ERich_geo.cpp origin/144-irt-geometry:src/PFRICH_geo.cpp
        git diff origin/irt-init-v01:compact/drich.xml origin/144-irt-geometry:compact/drich.xml
        git diff origin/irt-init-v01:compact/erich.xml origin/144-irt-geometry:compact/pfrich.xml
        git diff origin/irt-init-v01:compact/optical_materials.xml origin/144-irt-geometry:compact/optical_materials.xml
        git diff origin/irt-init-v01:CMakeLists.txt origin/144-irt-geometry:CMakeLists.txt
        ```
      - I have confirmed that these diffs are only the name change eRICH to pfRICH, along with the introduction of "lores" pfRICH sensor sizes, which are only used in CI visualization
- branch `drich-two-mirrors` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/260) (draft)
- branch `129-update-erich-name` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/328) (approved, not yet merged)

## `eicd`

## `Project Juggler`

## `benchmarks/reconstruction_benchmarks`
