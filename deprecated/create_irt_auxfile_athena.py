#!/usr/bin/env python
# produce auxiliary ROOT file for the IRT algorithm; assumes the
# production of `libIRT` objects is in the geometry `cpp` file
# 
# DEPRECATED for usage in EPIC; this script is preserved for 2 reasons:
# 1. usage for ATHENA support
# 2. example how to edit compact files on-the-fly with Python

import shutil, os, sys, argparse
import xml.etree.ElementTree as et

# arguments
parser = argparse.ArgumentParser()
parser.add_argument(
        '-o', '--output-name', dest='outFile', required=True,
        help='output auxiliary ROOT file name', type=str
        )
argv = parser.parse_args()

# import DDG4 after arg-parsing; it is slower
import DDG4

########################################

# compact files
if not 'DETECTOR_PATH' in os.environ:
    print('ERROR: env var DETECTOR_PATH not set',file=sys.stderr)
    exit(1)
mainFile = os.environ['DETECTOR_PATH'] + '/' + os.environ['DETECTOR'] + '.xml'
richFile = os.environ['DETECTOR_PATH'] + '/compact/drich.xml'

########################################

# backup original richFile, then parse
shutil.copy(richFile,richFile+'.bak')
richTree = et.parse(richFile)

# enable `DRICH_create_irt_file` mode
for constant in richTree.iter(tag='constant'):
    if(constant.attrib['name']=='DRICH_create_irt_file'):
        constant.set('value','1')

# set auxiliary file name
for detector in richTree.iter(tag='detector'):
    detector.set('irt_filename',argv.outFile)

# overwrite original richFile
richTree.write(richFile)

########################################

# produce IRT config file
try:
    kernel = DDG4.Kernel()
    kernel.loadGeometry(f'file:{mainFile}')
    kernel.terminate()
    print(f'\n -> produced {argv.outFile}\n')
except:
    pass

# revert to the original richFile
os.replace(richFile+'.bak',richFile)
