# Working Branches
These are the branches we used for the ATHENA proposal:

| Repository                  | New Branch                  | Old Proposal Branch |
| --:                         | ---                         | ---                 |
| `irt`                       | `irt-init-v02`              | `irt-init-v01`      |
| `ip6`                       | `master`                    | `master`            |
| `athena`                    | `144-irt-geometry`          | `irt-init-v01`      |
| `eicd`                      | `irt-data-model`            | `irt-init-v01`      |
| `juggler`                   | `73-add-rich-irt-algorithm` | `irt-init-v01`      |
| `reconstruction_benchmarks` | `irt-benchmark`             | `irt-benchmark`     |

### Commit DAGs
It is useful to watch the commit graphs (DAGs), for a visualization of the commits and branches:
- [`irt`](https://eicweb.phy.anl.gov/EIC/irt/-/network/main)
- [`ip6`](https://eicweb.phy.anl.gov/EIC/detectors/ip6/-/network/master)
- [`athena`](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/network/master)
- [`eicd`](https://eicweb.phy.anl.gov/EIC/eicd/-/network/master)
- [`juggler`](https://eicweb.phy.anl.gov/EIC/juggler/-/network/master)
- [`reconstruction_benchmarks`](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks/-/network/master)

Additional upstream DAGs to keep an eye on:
- [`NPDet`](https://eicweb.phy.anl.gov/EIC/NPDet/-/network/master) - expect more changes soon, especially in data model (`src/dd4pod`)

# Active Merge Requests and Issues

### `irt`
- BRANCH `irt-init-v02` - [MR](https://eicweb.phy.anl.gov/EIC/irt/-/merge_requests/8)
  - TODO: merge into `master`
  - this is `irt-init-v01`, plus the name change eRICH to pfRICH
- BRANCH `photomultiplierhit-to-trackerhit` - [MR](https://eicweb.phy.anl.gov/EIC/irt/-/merge_requests/9) (merged into `irt-init-v02`)
- BRANCH `erich-2-pfrich` - [MR](https://eicweb.phy.anl.gov/EIC/irt/-/merge_requests/7) (merged into `irt-init-v02`)

### `athena`
- BRANCH `144-irt-geometry` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/331)
  - TODO: this is the "backdoor" in frontdoor vs. backdoor (see below)
  - cloned and rebased from branch `irt-init-v01`; see [miscellaneous notes section below](#rebasenotes) for notes on validation of this procedure 
- BRANCH `129-update-erich-name` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/328) (merged)
- BRANCH `drich-two-mirrors` - [MR](https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/260) (draft)

### `eicd`
- BRANCH `irt-data-model` - [MR](https://eicweb.phy.anl.gov/EIC/eicd/-/merge_requests/70)
  - cloned and rebased from `irt-init-v01`

### `juggler`
- BRANCH `73-add-rich-irt-algorithm` - [MR](https://eicweb.phy.anl.gov/EIC/juggler/-/merge_requests/377)
  - cloned and rebased from `irt-init-v01`

### `reconstruction_benchmarks`
- BRANCH `irt-benchmark` - [MR](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks/-/merge_requests/222)
  - the options files from `irt` have been copied to this branch
    - CI environment variables have been added
    - verbosity level decreased (disable debugging statements)
