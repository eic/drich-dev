# Transitioning from Gitlab to Github

### Github access:
- Request to join [EIC organization](https://github.com/eic)
- Request to join [epic-devs](https://github.com/orgs/eic/teams/epic-devs)
  - Click "Members" tab, then "Request to Join"

### Repositories that have been transitioned to Github:
- [`ip6`](https://github.com/eic/ip6)
- [`ecce`](https://github.com/eic/ecce)

For each of your local clones of these repositories, re-configure remote URLs.
Assuming your remote is `origin` (check with `remote -v`), run the following
commands from _within_ the repository's directory:
```bash
pushd ecce && git remote set-url origin git@github.com:eic/ecce.git && popd
pushd ip6  && git remote set-url origin git@github.com:eic/ip6.git  && popd
```
These are URLs for SSH access; substitute them with HTTPS URLs if you prefer.

### For each merge request on Gitlab:
- the corresponding branch should already be on the Github remote (it was
  mirrored)
- open a new (draft) pull request on Github
- close your Gitlab merge request


# Name change to EPIC:
If we change the name of the `ecce` repository to `epic`, do the following:
```bash
mv ecce epic   # rename your local clone
pushd epic
git remote set-url origin git@github.com:eic/epic.git
popd
```
If instead we fork `ecce`, clone the new fork
