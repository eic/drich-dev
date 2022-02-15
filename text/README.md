# miscellaneous dRICH text files
- `materialTableDump.txt`: dump of material properties tables
  - obtained from [cisbani/dRICh](https://github.com/cisbani/dRICh) Fun4all port
  - possibly out of date, with respect to DD4hep port
- `sensorLUT.dat`: table of sensor positions
  - this LUT is used in `drawSegmentation.cpp` to draw a "map" of the pixel hits, useful for validating readout mapping
  - produced in dRICH `.cpp` file in DD4hep port (search for commented out LUT print statements)
  - possibly out of date, if so, update it 
