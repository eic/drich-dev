#!/usr/bin/env python
import ROOT as r
r.TGeoManager.Import("geo/detector_geometry.root")
r.gGeoManager.SetVisLevel(8)
drich = r.gGeoManager.GetVolume("DRICH")
drich.Draw("ogl")
# sensor = r.gGeoManager.GetVolume("DRICH_sensor_sec0")
# sensor.Draw("ogl")
