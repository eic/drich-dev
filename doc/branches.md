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

## IRT -- Legacy Juggler Support (DEPRECATED, replaced by EICrecon)
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
