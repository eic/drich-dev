# Transitioning from Gitlab to Github

This guide is meant for those who are already using `drich-dev`. If you
have started `drich-dev` from a "clean slate", this guide does not apply.

### Github access:
- Request to join [EIC organization](https://github.com/eic)
- Request to join [epic-devs](https://github.com/orgs/eic/teams/epic-devs)
  (Click "Members" tab, then "Request to Join)

### Repositories that have been transitioned to Github:
- [`ip6`](https://github.com/eic/ip6)
- [`ecce`](https://github.com/eic/ecce)

For each of your local clones of these repositories, re-configure remote URLs.
Run the following commands from the top-level `drich-dev` directory:
```bash
pushd ecce && git remote set-url origin git@github.com:eic/ecce.git && popd
pushd ip6  && git remote set-url origin git@github.com:eic/ip6.git  && popd
```
These are URLs for SSH access; substitute them with HTTPS URLs if you prefer.
This also assumes your remote repositories are named `origin` (very likely;
you can check with `git remote -v` from within each repository)

### Update drich-dev
The current `main` branch of `drich-dev` has all scripts and documentation
updated. In anticipation of the name change `ECCE` to `EPIC`, rename your
`ecce` directory to `epic`:
```bash
mv ecce epic
```
The build script `buildECCE.sh` has been renamed to `buildEPIC.sh`, and now
points to the `epic` directory.

See below for steps to take when the name of the `ecce` repository is changed.


### For each merge request on Gitlab:
- the corresponding branch should already be on the Github remote (it was
  mirrored)
- open a new (draft) pull request on Github
- close your Gitlab merge request


# Name change to EPIC:
If we change the name of the `ecce` repository to `epic`, do the following:

Rename your local clone:
```bash
mv ecce epic
```

Re-configure the remote URL:
```bash
pushd epic
git remote set-url origin git@github.com:eic/epic.git
popd
```

If instead we fork `ecce`, clone the new fork
