# Working Branches

The following tables list `git` branches for each repository. Each table is for a 
project or recommended configuration. Links to corresponding merge requests are provided.
We intend to keep these tables up-to-date as development proceeds.

## Production
| Repository                  | Branch   |
| --:                         | ---      |
| `drich-dev`                 | `main`   |
| `ip6`                       | `master` |
| `ecce`                      | `main`   |
| `eicd`                      | `master` |
| `irt`                       | `main`   |
| `juggler`                   | `master` |
| `reconstruction_benchmarks` | `master` |

## Envelope Updates
| Repository                  | Branch                                                                                                                                  |
| --:                         | ---                                                                                                                                     |
| `drich-dev`                 | `main`                                                                                                                                  |
| `ip6`                       | `master`                                                                                                                                |
| `ecce`                      | `main` vs. [`18-drich-increase-z-length-and-add-space-for-services`](https://eicweb.phy.anl.gov/EIC/detectors/ecce/-/merge_requests/33) |
| `eicd`                      | `master`                                                                                                                                |
| `irt`                       | `main`                                                                                                                                  |
| `juggler`                   | `master`                                                                                                                                |
| `reconstruction_benchmarks` | `master`                                                                                                                                |

## IRT Development
| Repository                  | Branch                                                                                                       |
| --:                         | ---                                                                                                          |
| `drich-dev`                 | `dev-irt`                                                                                                    |
| `ip6`                       | `master`                                                                                                     |
| `ecce`                      | [`17-drich-produce-irt-geometry-objects`](https://eicweb.phy.anl.gov/EIC/detectors/ecce/-/merge_requests/31) |
| `eicd`                      | [`irt-data-model`](https://eicweb.phy.anl.gov/EIC/eicd/-/merge_requests/70)                                  |
| `irt`                       | `main` or [`edm4hep-refactoring`](https://eicweb.phy.anl.gov/EIC/irt/-/merge_requests/10)                    |
| `juggler`                   | [`73-add-rich-irt-algorithm`](https://eicweb.phy.anl.gov/EIC/juggler/-/merge_requests/377)                   |
| `reconstruction_benchmarks` | [`irt-benchmark`](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks/-/merge_requests/222)  |

## Sensor Development
| Repository                  | Branch                                                                                                                   |
| --:                         | ---                                                                                                                      |
| `drich-dev`                 | `main`                                                                                                                   |
| `ip6`                       | `master`                                                                                                                 |
| `ecce`                      | [`12-drich-sensor-material-should-not-be-airoptical`](https://eicweb.phy.anl.gov/EIC/detectors/ecce/-/merge_requests/28) |
| `eicd`                      | `master`                                                                                                                 |
| `irt`                       | `main`                                                                                                                   |
| `juggler`                   | `master`                                                                                                                 |
| `reconstruction_benchmarks` | `master`                                                                                                                 |


# Commit DAGs
It is useful to watch the commit graphs (DAGs), for a visualization of the commits and branches:
- [`drich-dev`](https://github.com/c-dilks/drich-dev/network)
- [`ip6`](https://eicweb.phy.anl.gov/EIC/detectors/ip6/-/network/master)
- [`ecce`](https://eicweb.phy.anl.gov/EIC/detectors/ecce/-/network/master)
- [`eicd`](https://eicweb.phy.anl.gov/EIC/eicd/-/network/master)
- [`irt`](https://eicweb.phy.anl.gov/EIC/irt/-/network/main)
- [`juggler`](https://eicweb.phy.anl.gov/EIC/juggler/-/network/master)
- [`reconstruction_benchmarks`](https://eicweb.phy.anl.gov/EIC/benchmarks/reconstruction_benchmarks/-/network/master)

Additional upstream DAGs to keep an eye on:
- [`EDM4hep`](https://github.com/key4hep/EDM4hep/network)
- [`NPDet`](https://eicweb.phy.anl.gov/EIC/NPDet/-/network/master) - includes (older) `dd4pod` data model (`src/dd4pod`)
