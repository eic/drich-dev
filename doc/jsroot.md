### Locally Hosting JSroot

It is convenient to locally run JSroot, so you can automate opening and viewing
the geometry. Follow the [JSroot documentation](https://github.com/root-project/jsroot/blob/master/docs/JSROOT.md)
to learn how to set custom settings with URLs, and much more.

First either obtain a release or clone the JSroot repository; you can
clone it to any directory (does not have to be in `drich-dev/`)
```
git clone https://github.com/root-project/jsroot.git
```
Then `cd` to this `jsroot` directory. Make a symlink to the `drich-dev/geo`
directory, so that your local HTTP server can access ROOT files within:
```
ln -sv /path/to/drich-dev/geo ./
```
Now start an HTTP server. For example, using python:
```
python -m http.server
```
Note which port is used, likely `8000`. Now open your browser and open the URL
<http://localhost:8000> to start JSroot in the browser (change the port number
if yours is different) 

Various settings can be set via the URL. For example, the following URL
automatically opens the `detector_geometry.root` file (produced by
`geometry.sh`) using `file=...`, and enables dark mode:

<http://localhost:8000/?file=geo/detector_geometry.root&dark>
