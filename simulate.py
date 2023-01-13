#!/usr/bin/env python

# -----------------------------------------------#
# npsim wrapper with EIC RICH specific tests     #
# Author: C. Dilks                               #
# -----------------------------------------------#

import sys, getopt, os, re, importlib
import pprint
import subprocess, shlex
import math
from numpy import linspace

# SETTINGS
################################################################
use_npdet_info = False  # use np_det_info to get envelope dimensions
rMinBuffer = 5  # acceptance test rMin = vessel rMin + rMinBuffer [cm]
rMaxBuffer = 5  # acceptance test rMax = vessel rMax - rMinBuffer [cm]

# ARGUMENTS
################################################################

inputFileName = ''
testNum = -1
standalone = False
compactFileCustom = ''
zDirection = 1
particle = 'pi+'
energy = '40.0 GeV'
runType = 'run'
numEvents = 50
numTestSamples = 0
restrict_sector = True
outputImageType = ''
outputFileName = ''
useEDM4hepFormat = True

helpStr = f'''
{sys.argv[0]} <INPUT_FILE or TEST_NUM> [OPTIONS]

<REQUIRED ARGUMENTS>: provide either an INPUT_FILE or a TEST_NUM

    INPUT_FILE: -i <input file>: specify an input file, e.g., hepmc

    TEST_NUM:  -t <testnum>: specify which test to run
            >> acceptance tests:
                1: aim pions at center of aerogel sector
                2: inner edge test
                3: outer edge test
                4: polar scan test
                5: azimuthal + polar scan test
                6: spray pions in one sector
                7: momentum scan, for aerogel
                8: momentum scan, for gas
            >> optics tests:
                10:   focal point, in RICH acceptance
                        ( recommend: optDbg=1 / mirDbg=0 / sensDbg=1 )
                11:   focal point, broad range test
                        ( recommend: optDbg=1 / mirDbg=1 / sensDbg=1 )
                12:   parallel-to-point focal test
                        ( recommend: optDbg=1 / mirDbg=0 / sensDbg=0 )
                13:   evenly distributed sensor hits test
                        ( recommend: optDbg=3 / mirDbg=0 / sensDbg=0 )

[OPTIONAL ARGUMENTS]

    OPTIONS:    -d: direction to throw particles (may not be used by all tests)
                    1 = toward dRICH (default)
                   -1 = toward pfRICH
                -s: enable standalone RICH-only simulation (default is full detector)
                -c [compact file]: specify a custom compact file
                   (this will override -d and -s options)
                -p [particle]: name of particle to throw; default: {particle}
                   examples:
                    - e- / e+
                    - pi+ / pi-
                    - kaon+ / kaon-
                    - proton / anti_proton
                    - opticalphoton
                -e [energy]: energy (GeV) for mono-energetic runs (default={energy})
                -n [numEvents]: number of events to process (default={numEvents})
                   (if using TEST_NUM, this is usually the number of events PER fixed momentum)
                -k [numTestSamples]: some tests throw particles in multiple different directions,
                   such as "polar scan test"; for this test, use [numTestSamples] to control
                   how many directions are tested
                   - many tests offer a similar usage of [numTestSamples]
                   - these tests also have default [numTestSamples] values
                -a: allow azimuthal scans to cover the full 2*pi range, rather than restricting
                    to a single sector
                -r: run, instead of visualize (default)
                -v: visualize, instead of run
                -m [output image type]: save visual with specified type (svg,pdf,ps)
                   - useful tip: if you want to suppress the drawing of the visual, but
                     still save an output image, use Xvbf (start EIC container shell
                     as `xvfb-run eic-shell`); this is good for batch processing
                -o [output file]: output root file name (overrides any default name)
                -f: use TTree output format, rather than the default EDM4hep format, which
                    is a TTree with PODIO metadata. The EDM4hep format is required
                    for downstream reconstruction code, whereas the '-f' option produces
                    a file which is easier to view in a TBrowser
    '''

if (len(sys.argv) <= 1):
    print(helpStr)
    sys.exit(2)
try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:t:d:sc:p:e:n:k:arvm:o:f')
except getopt.GetoptError:
    print('\n\nERROR: invalid argument\n', helpStr)
    sys.exit(2)
for opt, arg in opts:
    if (opt == '-i'): inputFileName = arg.lstrip()
    if (opt == '-t'): testNum = int(arg)
    if (opt == '-d'): zDirection = int(arg)
    if (opt == '-s'): standalone = True
    if (opt == '-c'): compactFileCustom = arg.lstrip()
    if (opt == '-p'): particle = arg.lstrip()
    if (opt == '-e'): energy = arg.lstrip() + " GeV"
    if (opt == '-n'): numEvents = int(arg)
    if (opt == '-k'): numTestSamples = int(arg)
    if (opt == '-a'): restrict_sector = False
    if (opt == '-r'): runType = 'run'
    if (opt == '-v'): runType = 'vis'
    if (opt == '-m'): outputImageType = arg.lstrip()
    if (opt == '-o'): outputFileName = arg.lstrip()
    if (opt == '-f'): useEDM4hepFormat = False
if (testNum < 0 and inputFileName == ''):
    print('\n\nERROR: Please specify either an input file (`-i`) or a test number (`-t`).\n', helpStr)
    sys.exit(2)
elif (testNum > 0 and inputFileName != ''):
    print('\n\nWARNING: You specified both an input file and a test number; proceeding with the input file only.\n')
    testNum = -1

### overrides
if (testNum >= 10):
    print("optics test, overriding some settings...")
    particle = 'opticalphoton'
    standalone = True
    if (testNum in [10,11,12]):
        print("-- this is a visual test --")
        runType = 'vis'
if (particle == "opticalphoton"):
    energy = '3.0 eV'
    print(f'optical photons test: using energy {energy}')

### configure input and output file names
### relative paths will be made absolute here
workDir = os.getcwd()
##### ensure input file name has absolute path
if inputFileName != '':
    if not bool(re.search('^/', inputFileName)): inputFileName = workDir + "/" + inputFileName
##### ensure output file name has absolute path (and generate default name, if unspecified)
if outputFileName == '':
    outputFileName = workDir + "/out/sim.root"  # default name
elif not bool(re.search('^/', outputFileName)):
    outputFileName = workDir + "/" + outputFileName  # convert relative path to absolute path
##### get output file basename
outputName = re.sub('\.root$', '', outputFileName)
outputName = re.sub('^.*/', '', outputName)
##### set output file name for `npsim`, which is sensitive to file extension
outputFileName_npsim = outputFileName
if useEDM4hepFormat:
    outputFileName_npsim = re.sub('\.root$', '.edm4hep.root', outputFileName_npsim)

### set RICH names, based on zDirection
zDirection /= abs(zDirection)
if (zDirection < 0):
    xrich = 'pfrich'
    XRICH = 'PFRICH'
    xRICH = 'pfRICH'
else:
    xrich = 'drich'
    XRICH = 'DRICH'
    xRICH = 'dRICH'

### get env vars

detMain = os.environ['DETECTOR']
detPath = os.environ['DETECTOR_PATH']
outDir  = os.environ['DRICH_DEV'] + '/out'

### set compact file
compactFileFull = detPath + '/' + detMain + '.xml'
compactFileRICH = detPath + '/' + detMain + '_' + xrich + '_only.xml'
compactFile = compactFileRICH if standalone else compactFileFull
if compactFileCustom != '':
    if not bool(re.search('^/', compactFileCustom)):
        compactFileCustom = workDir + "/" + compactFileCustom  # convert relative path to absolute path
    compactFile = compactFileCustom

### print args and settings
sep = '-' * 40
print(sep)
print("** simulation args **")
print(f'inputFileName  = {inputFileName}')
print(f'testNum        = {testNum}')
print(f'particle       = {particle}')
print(f'numEvents      = {numEvents}')
print(f'numTestSamples = {numTestSamples}')
print(f'runType        = {runType}')
print(f'direction      = toward {xRICH}')
print(f'outputFileName = {outputFileName}')
print(f'outputName     = {outputName}')
print(f'compactFile    = {compactFile}')
print(sep)

# SETTINGS AND CONFIGURATION
################################################################

### start macro file
m = open(workDir + "/macro/macro_" + outputName + ".mac", 'w+')

### common settings
m.write(f'/control/verbose 2\n')
m.write(f'/run/initialize\n')
# m.write(f'/run/useMaximumLogicalCores\n')

### visual settings
if (runType == 'vis'):
    m.write(f'/vis/open OGL 800x800-0+0\n')  # driver
    m.write(f'/vis/scene/create\n')
    m.write(f'/vis/scene/add/volume\n')
    m.write(f'/vis/scene/add/axes 0 0 0 1 m\n')
    m.write(f'/vis/scene/add/trajectories smooth\n')
    m.write(f'/vis/scene/add/hits\n')
    m.write(f'/vis/sceneHandler/attach\n')
    # m.write(f'/vis/viewer/set/viewpointThetaPhi 115 65\n') # angled view
    # m.write(f'/vis/viewer/set/viewpointThetaPhi 0 0\n') # front view
    m.write(f'/vis/viewer/set/viewpointThetaPhi -90 -89\n')  # top view
    # m.write(f'/vis/viewer/set/viewpointThetaPhi 90 0\n') # side view
    # m.write(f'/vis/viewer/zoom 0.5\n')
    m.write(f'/vis/viewer/set/style wireframe\n')
    m.write(f'/vis/modeling/trajectories/create/drawByCharge\n')
    m.write(f'/vis/modeling/trajectories/drawByCharge-0/setRGBA 0 0.8 0 0 1\n')
    m.write(f'/vis/modeling/trajectories/drawByCharge-0/setRGBA 1 0 0.5 0.5 1\n')

### append particle info
m.write(f'/gps/verbose 2\n')
m.write(f'/gps/particle {particle}\n')
m.write(f'/gps/number 1\n')
if (testNum != 7 and testNum != 8): m.write(f'/gps/ene/mono {energy}\n')
# m.write(f'/gps/ene/type Gauss\n')
# m.write(f'/gps/ene/sigma 3.0 GeV\n')

### append source settings
m.write(f'/gps/position 0 0 0 cm\n')

# ACCEPTANCE LIMITS
################################################################

### RICH envelope parameters
params = {}
if detMain=='athena':
    print('This is ATHENA, calling npdet_info to determine acceptance limits')
    use_npdet_info = True
if use_npdet_info:
    ### call `npdet_info` to obtain most up-to-date RICH attributes and values
    paramListFileN = f'{outDir}/params_{outputName}.txt'
    with open(paramListFileN, 'w') as paramListFile:
        cmd = f'npdet_info search {XRICH} --value {compactFileFull}'
        print(sep)
        print('EXECUTE: ' + cmd)
        print(sep)
        subprocess.call(shlex.split(cmd), stdout=paramListFile)
    for paramLine in open(paramListFileN, 'r'):
        print(paramLine)
        paramLineKV = paramLine.strip().split('=')
        if (len(paramLineKV) == 2): 
            try:
                params.update({paramLineKV[0].strip(): float(paramLineKV[1].strip())})
            except ValueError:
                pass # ignore string constants
else:
    ### hard-coded values (faster and reliable, but maybe out of date)
    # dRICH:
    params['DRICH_rmin1'] = 15.332
    params['DRICH_rmax2'] = 180.0
    params['DRICH_zmin']  = 195.0
    params['DRICH_zmax']  = 315.0
    # pfRICH
    params['PFRICH_rmin1'] = 5.945
    params['PFRICH_rmax']  = 63.0
    params['PFRICH_zmin']  = -118.6
    params['PFRICH_proximity_gap'] = 30.0
    params['PFRICH_aerogel_thickness'] = 3.0

### set envelope limits
if (zDirection < 0):
    rMin = params['PFRICH_rmin1'] + rMinBuffer
    rMax = params['PFRICH_rmax'] - rMaxBuffer
    zMax = -1*params['PFRICH_zmin'] + params['PFRICH_aerogel_thickness'] + params['PFRICH_proximity_gap']  # must be positive
else:
    rMin = params['DRICH_rmin1'] + rMinBuffer
    rMax = params['DRICH_rmax2'] - rMaxBuffer
    zMax = params['DRICH_zmax']
print('** constants from DD4hep **')
pprint.pprint(params)
print(sep)
print('** acceptance limits **')
print(f'rMin = {rMin} cm')
print(f'rMax = {rMax} cm')
print(f'zMax = {zMax} cm')

### set angular acceptance limits
thetaMin = math.atan2(rMin, zMax)
thetaMax = math.atan2(rMax, zMax)
etaMin = -math.log(math.tan(0.5 * thetaMax))
etaMax = -math.log(math.tan(0.5 * thetaMin))
print(f'thetaMin = {math.degrees(thetaMin)} deg')
print(f'thetaMax = {math.degrees(thetaMax)} deg')
print(f'etaMin = {etaMin}')
print(f'etaMax = {etaMax}')
print(sep)

# TEST SETTINGS
######################################

### `switch testNum:`

if testNum == 1:
    m.write(f'\n# aim at +x {xRICH} sector\n')
    thetaMid = (thetaMin+thetaMax)/2.0
    x = math.sin(thetaMid)
    y = 0.0
    z = math.cos(thetaMid) * zDirection
    m.write(f'/gps/direction {x} {y} {z}\n')
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 2:
    m.write(f'\n# inner edge of acceptance\n')
    x = math.sin(thetaMin)
    y = 0.0
    z = math.cos(thetaMin) * zDirection
    m.write(f'/gps/direction {x} {y} {z}\n')
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 3:
    m.write(f'\n# outer edge of acceptance\n')
    x = math.sin(thetaMax)
    y = 0.0
    z = math.cos(thetaMax) * zDirection
    m.write(f'/gps/direction {x} {y} {z}\n')
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 4:
    numTheta = 4 if numTestSamples==0 else numTestSamples  # number of theta steps
    m.write(f'\n# polar scan test\n')
    if (runType == "vis"):
        m.write(f'/vis/scene/endOfEventAction accumulate\n')
        m.write(f'/vis/scene/endOfRunAction accumulate\n')
    for theta in list(linspace(thetaMin, thetaMax, numTheta)):
        x = math.sin(theta)
        y = 0.0
        z = math.cos(theta) * zDirection
        m.write(f'/gps/direction {x} {y} {z}\n')
        m.write(f'/run/beamOn {numEvents}\n')
        # m.write(f'/gps/direction -{x} {y} {z}\n') # include -x sector
        # m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 5:
    numTheta = 4 if numTestSamples==0 else numTestSamples # number of theta steps
    numPhi = 24  # number of phi steps, prefer even multiple of 6 (12,24,36) to check sector boundaries
    m.write(f'\n# polar+azimuthal scan test\n')
    if (runType == "vis"):
        m.write(f'/vis/scene/endOfEventAction accumulate\n')
        m.write(f'/vis/scene/endOfRunAction accumulate\n')
    print(f'SET theta range to {math.degrees(thetaMin)} to {math.degrees(thetaMax)} deg')
    for theta in list(linspace(thetaMin, thetaMax, numTheta)):
        for phi in list(linspace(0, 2 * math.pi, numPhi, endpoint=False)):
            if restrict_sector and (math.pi / 6 < phi < (2 * math.pi - math.pi / 6)): continue  # restrict to one sector
            if (abs(phi) > 0.001 and abs(theta - thetaMin) < 0.001): continue  # allow only one ring at thetaMin
            x = math.sin(theta) * math.cos(phi)
            y = math.sin(theta) * math.sin(phi)
            z = math.cos(theta) * zDirection
            m.write(f'/gps/direction {x} {y} {z}\n')
            m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 6:
    m.write(f'\n# pion spray test, {xRICH} range\n')  # TODO: probably broken
    if (runType == "vis"):
        m.write(f'/vis/scene/endOfEventAction accumulate\n')
    m.write(f'/gps/pos/type Point\n')
    m.write(f'/gps/pos/radius 0.1 mm\n')
    m.write(f'/gps/ang/type iso\n')
    m.write(f'/gps/ang/mintheta {math.pi - thetaMax} rad\n')
    m.write(f'/gps/ang/maxtheta {math.pi - thetaMin} rad\n')
    m.write(f'/gps/ang/minphi {math.pi} rad\n')
    m.write(f'/gps/ang/maxphi {math.pi + 0.01} rad\n')
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 7 or testNum == 8:
    numMomPoints = 10 if numTestSamples==0 else numTestSamples # number of momenta
    m.write(f'\n# momentum scan\n')
    thetaMid = (thetaMin+thetaMax)/2.0
    x = math.sin(thetaMid)
    y = 0.0
    z = math.cos(thetaMid) * zDirection
    m.write(f'/gps/direction {x} {y} {z}\n')
    momMax = 60
    if testNum == 7:
        momMax = 20
    for en in list(linspace(1, momMax, numMomPoints)):
        m.write(f'/gps/ene/mono {en} GeV\n')
        m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 10:
    m.write(f'\n# opticalphoton scan test, {xRICH} range\n')
    m.write(f'/vis/scene/endOfEventAction accumulate\n')
    m.write(f'/gps/pos/type Point\n')
    m.write(f'/gps/pos/radius 0.1 mm\n')
    m.write(f'/gps/ang/type iso\n')
    m.write(f'/gps/ang/mintheta {math.pi - thetaMax} rad\n')
    m.write(f'/gps/ang/maxtheta {math.pi - thetaMin} rad\n')
    m.write(f'/gps/ang/minphi {math.pi} rad\n')
    m.write(f'/gps/ang/maxphi {math.pi + 0.01} rad\n')
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 11:
    m.write(f'\n# opticalphoton scan test, broad range\n')
    m.write(f'/vis/scene/endOfEventAction accumulate\n')
    m.write(f'/gps/pos/type Point\n')
    m.write(f'/gps/pos/radius 0.1 mm\n')
    m.write(f'/gps/ang/type iso\n')
    m.write(f'/gps/ang/mintheta {math.pi / 2} rad\n')
    m.write(f'/gps/ang/maxtheta {math.pi - thetaMin} rad\n')
    m.write(f'/gps/ang/minphi {math.pi} rad\n')
    m.write(f'/gps/ang/maxphi {math.pi + 0.01} rad\n')
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 12:
    numBeams = 5 if numTestSamples==0 else numTestSamples  # number of beams within theta acceptance
    m.write(f'\n# opticalphoton parallel-to-point focusing\n')
    #m.write(f'/vis/scene/endOfEventAction accumulate\n')
    #m.write(f'/vis/scene/endOfRunAction accumulate\n')
    m.write(f'/gps/pos/type Beam\n')
    m.write(f'/gps/ang/type beam1d\n')
    for rVal in list(linspace(rMin, rMax, numBeams)):
        m.write(f'/gps/ang/rot1 -{zMax} 0 {rVal}\n')
        m.write(f'/gps/pos/rot1 -{zMax} 0 {rVal}\n')
        m.write(f'/gps/pos/halfx 16 cm\n')  # parallel beam width
        m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 13:
    m.write(f'\n# evenly distributed sensor hits test\n')
    if runType == "vis":
        m.write(f'/vis/scene/endOfEventAction accumulate\n')
        m.write(f'/vis/scene/endOfRunAction accumulate\n')

    from scripts import createAngles
    num_rings = 120 if numTestSamples==0 else numTestSamples  # number of concentric rings, type=int
    hit_density = 80  # amount of photon hits for the smallest polar angle, type=int
    angles = createAngles.makeAngles(thetaMin, thetaMax, num_rings, hit_density)  # list of angles

    print(f'SET theta range to {math.degrees(thetaMin)} to {math.degrees(thetaMax)} deg')
    for angle in angles:
        theta, phi = angle[0], angle[1]
        if restrict_sector and (math.pi / 6 < phi < (2 * math.pi - math.pi / 6)): continue  # restrict to one sector
        if abs(phi) > 0.001 and abs(theta - thetaMin) < 0.001: continue  # allow only one ring at thetaMin
        x = math.sin(theta) * math.cos(phi)
        y = math.sin(theta) * math.sin(phi)
        z = math.cos(theta) * zDirection
        m.write(f'/gps/direction {x} {y} {z}\n')
        m.write(f'/run/beamOn {numEvents}\n')
        
elif testNum == 14:
    m.write(f'\n# opticalphoton parallel-to-point focusing, full coverage\n')
    #m.write(f'/vis/scene/endOfEventAction accumulate\n')
    #m.write(f'/vis/scene/endOfRunAction accumulate\n')
    m.write(f'/gps/pos/type Beam\n')
    m.write(f'/gps/ang/type beam1d\n')
    import numpy as np
    def makeBasicAngles(theta_min, theta_max, num_theta, num_phi):
        angles = []
        thetas = np.linspace(theta_min, theta_max, num=num_theta)
        phis = np.linspace(-math.pi/6, math.pi/6, num=num_phi)
        for i in range(num_phi):
            if phis[i] < 0:
                phis[i] = phis[i]+2*np.pi
        for i in thetas:
            for j in phis:
                angles.append(tuple((i,j)))
        return angles
    angles = makeBasicAngles(thetaMin, thetaMax, 50, 50)

    for angle in angles:
        theta, phi = angle[0], angle[1]
        if math.pi / 6 < phi < (2 * math.pi - math.pi / 6): continue  # restrict to one sector                                                                                                  
        x = math.sin(theta) * math.cos(phi)
        y = math.sin(theta) * math.sin(phi)
        z = math.cos(theta) * zDirection
        m.write(f'/gps/direction {x} {y} {z} \n')
        m.write(f'/gps/pos/halfx 16 cm\n')  # parallel beam width                                                                                                                                          
        m.write(f'/run/beamOn {numEvents}\n')

elif testNum > 0:
    print("ERROR: unknown test number\n")
    m.close()
    sys.exit(2)

### finalize
if (runType == "vis"):
    m.write(f'/vis/viewer/flush\n')
    m.write(f'/vis/viewer/refresh\n')
    if outputImageType!='':
        m.write(f'/vis/ogl/export {re.sub("root$",outputImageType,outputFileName)}\n')

### print macro and close stream
m.seek(0, 0)
if (testNum > 0):
    print(m.read())
m.close()

# RUN npsim
#########################################################

### simulation executable and arguments
cmd = [
        f'npsim',
        # f'{localDir}/NPDet/install/bin/npsim', # call local npsim
        f'--runType {runType}',
        f'--compactFile {compactFile}',
        f'--outputFile {outputFileName_npsim}',
        "--part.userParticleHandler=''", # necessary for opticalphotons truth output
        # '--random.seed 1',
        # '--part.keepAllParticles True',
        ]
if (testNum > 0):
    cmd.extend([
        f'--macro {m.name}',
        '--enableG4GPS',
        ])
else:
    cmd.extend([
      f'-N {numEvents}',
      f'--inputFiles \'{inputFileName}\'',
      ])

### run simulation
cmdShell = shlex.split(" ".join(cmd))
print(f'{sep}\nRUN SIMULATION:\n{shlex.join(cmdShell)}\n{sep}')
subprocess.run(cmdShell, cwd=detPath)

### correct the output file name to the specified name
os.rename(outputFileName_npsim, outputFileName)

### cleanup
# os.remove(m.name) # remove macro
print("\nPRODUCED SIMULATION OUTPUT FILE: " + outputFileName)
