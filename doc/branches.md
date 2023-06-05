# Working Branches

The following tables list `git` branches for each repository. Each table is for a 
project or recommended configuration. Links to corresponding pull requests are provided.
We intend to keep these tables up-to-date as development proceeds.

## Production
| Repository                  | Branch   |
| --:                         | ---      |
| `drich-dev`                 | `main`   |
| `epic`                      | `main`   |
| `EDM4eic`                   | `main`   |
| `irt`                       | `main`   |
| `EICrecon`                  | `main`   |
| `reconstruction_benchmarks` | `master` |

## IRT -- EICrecon development
| Repository                  | Branch                             | Pull Request                                                                                       |
| --:                         | ---                                | ---                                                                                                |
| `drich-dev`                 | `main`                             |                                                                                                    |
| `epic`                      | `main`                             |                                                                                                    |
| `EDM4eic`                   | `main`                             |                                                                                                    |
| `irt`                       | `main`                             |                                                                                                    |
| `EICrecon`                  | `irt-algo-stable` (or `irt-algo`)  | https://github.com/eic/EICrecon/pull/393                                                           |
| `reconstruction_benchmarks` | `irt-algo`                         | [MR 293](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks/-/merge_requests/293) |

- the branch `irt-algo-stable` is for a stable, working version; the branch
  `irt-algo` is for the unstable, most up-to-date version

## IRT -- Legacy Juggler Support
| Repository  | Branch                      | Pull Request                                                                |
| --:         | ---                         | ---                                                                         |
| `drich-dev` | `main`                      |                                                                             |
| `epic`      | `main`                      |                                                                             |
| `EDM4eic`   | `irt-data-model`            | https://github.com/eic/EDM4eic/pull/1                                       |
| `irt`       | `main`                      |                                                                             |
| `EICrecon`  | `main`                      |                                                                             |
| `juggler`   | `73-add-rich-irt-algorithm` | [MR at EICweb](https://eicweb.phy.anl.gov/EIC/juggler/-/merge_requests/377) |

NOTE: do not build `EICrecon`, since for Juggler support we only need the
`IrtGeo*` classes which are built externally here in `drich-dev`.
