#!/usr/bin/env python

# -----------------------------------------------#
# npsim wrapper with EIC RICH specific tests    #
# Author: C. Dilks                              #
# -----------------------------------------------#

import sys, getopt, os, re, importlib
import subprocess, shlex
import math
from numpy import linspace

# ARGUMENTS
################################################################

inputFileName = ''
testNum = -1
standalone = False
zDirection = 1
particle = 'pi+'
energy = '8.0 GeV'
runType = 'run'
numEvents = 10
outputFileName = ''

helpStr = f'''
{sys.argv[0]} <INPUT_FILE or TEST_NUM> [OPTIONS]

<REQUIRED ARGUMENTS>: provide either an INPUT_FILE or a TEST_NUM

    INPUT_FILE: -i <input file>: specify an input file, e.g., hepmc

    TEST_NUM:  -t <testnum>: specify which test to run
            >> acceptance tests:
                1: aim pions at center of aerogel sector
                2: inner edge test
                3: outer edge test
                4: radial scan test
                5: azimuthal+radial scan test (cf. test 8)
                6: spray pions in one sector
                7: momentum scan
                8: azimuthal+polar scan test (cf. test 5)
            >> optics tests:
                10:   focal point, in RICH acceptance
                        ( recommend: optDbg=1 / mirDbg=0 / sensDbg=1 )
                11:   focal point, broad range test
                        ( recommend: optDbg=1 / mirDbg=1 / sensDbg=1 )
                12:   parallel-to-point focal test
                        ( recommend: optDbg=1 / mirDbg=0 / sensDbg=0 )
                13:   evenly distributed sensor hits test

[OPTIONAL ARGUMENTS]

    OPTIONS:    -d: direction to throw particles (may not be used by all tests)
                    1 = toward positive (hadron) endcap RICH (default)
                   -1 = toward negative (electron) endcap RICH
                -s: enable standalone RICH-only simulation (default is full detector)
                -p [particle]: name of particle to throw; default: {particle}
                   examples:
                    - e- / e+
                    - pi+ / pi-
                    - kaon+ / kaon-
                    - proton / anti_proton
                    - opticalphoton
                -n [numEvents]: number of events to process (default={numEvents})
                   (if using TEST_NUM, this is usually the number of events PER fixed momentum)
                -e [energy]: energy (GeV) for mono-energetic runs (default={energy} GeV)
                -r: run, instead of visualize (default)
                -v: visualize, instead of run
                -o [output file]: absolute path output root file name (overrides any default name)
    '''

if (len(sys.argv) <= 1):
    print(helpStr)
    sys.exit(2)
try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:t:d:sp:n:e:rvo:')
except getopt.GetoptError:
    print('\n\nERROR: invalid argument\n', helpStr)
    sys.exit(2)
for opt, arg in opts:
    if (opt == '-i'): inputFileName = arg
    if (opt == '-t'): testNum = int(arg)
    if (opt == '-d'): zDirection = int(arg)
    if (opt == '-s'): standalone = True
    if (opt == '-p'): particle = arg
    if (opt == '-n'): numEvents = int(arg)
    if (opt == '-e'): energy = arg + " GeV"
    if (opt == '-r'): runType = 'run'
    if (opt == '-v'): runType = 'vis'
    if (opt == '-o'): outputFileName = arg
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
    outputFileName = workDir + "/sim_rich_" + runType + ".root"  # default name
elif not bool(re.search('^/', outputFileName)):
    outputFileName = workDir + "/" + outputFileName  # convert relative path to absolute path
##### get output file basename
outputName = re.sub('\.root$', '', outputFileName)
outputName = re.sub('^.*/', '', outputName)

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

detPath = os.environ['DETECTOR_PATH']
detMain = os.environ['JUGGLER_DETECTOR']
localDir = os.environ['LOCAL_DATA_PATH']

### set compact file
compactFileFull = detPath + '/' + detMain + '.xml'
compactFileRICH = detPath + '/' + detMain + '_' + xrich + '_only.xml'
compactFile = compactFileRICH if standalone else compactFileFull

### print args and settings
sep = '-' * 40
print(sep)
print("** simulation args **")
print(f'inputFileName  = {inputFileName}')
print(f'testNum        = {testNum}')
print(f'particle       = {particle}')
print(f'numEvents      = {numEvents}')
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
    m.write(f'/vis/open OGLSQt 800x800-0+0\n')  # driver
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
if (testNum != 7): m.write(f'/gps/ene/mono {energy}\n')
# m.write(f'/gps/ene/type Gauss\n')
# m.write(f'/gps/ene/sigma 3.0 GeV\n')

### append source settings
m.write(f'/gps/position 0 0 0 cm\n')

# ACCEPTANCE LIMITS
################################################################

### call `npdet_info` to obtain RICH attributes and values
paramListFileN = f'{localDir}/params_{outputName}.txt'
with open(paramListFileN, 'w') as paramListFile:
    cmd = f'npdet_info search {XRICH} --value {compactFileFull}'
    print(sep)
    print('EXECUTE: ' + cmd)
    print(sep)
    subprocess.call(shlex.split(cmd), stdout=paramListFile)
params = {}
for paramLine in open(paramListFileN, 'r'):
    print(paramLine)
    paramLineKV = paramLine.strip().split('=')
    if (len(paramLineKV) == 2): params.update({paramLineKV[0].strip(): float(paramLineKV[1].strip())})

### set envelope limits
envBuffer = 5
if (zDirection < 0):
    rMin = params['PFRICH_rmin1'] + envBuffer
    rMax = params['PFRICH_rmax'] - envBuffer
    zMax = params['PFRICH_zmax'] * -1 - 20  # must be positive; subtract 20 since sensors are not at `zmax`
    # TODO: use instead `params['PFRICH_sensor_dist']` `when https://eicweb.phy.anl.gov/EIC/detectors/athena/-/merge_requests/290` is merged
else:
    rMin = params['DRICH_rmin1'] + envBuffer
    rMax = params['DRICH_rmax2'] - envBuffer
    zMax = params['DRICH_zmin'] + params['DRICH_Length']
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
    m.write(f'/gps/direction 0.35 0.0 {zDirection}\n')
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 2:
    m.write(f'\n# inner edge of acceptance\n')
    if (zDirection < 0):
        m.write(f'/gps/direction {math.sin(math.radians(2.4))} 0.0 -{math.cos(math.radians(2.4))}\n')
    else:
        m.write(f'/gps/direction {math.sin(math.radians(2.9))} 0.0 {math.cos(math.radians(2.9))}\n')
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 3:
    m.write(f'\n# outer edge of acceptance\n')
    if (zDirection < 0):
        m.write(f'/gps/direction {math.sin(math.radians(25.0))} 0.0 -{math.cos(math.radians(25.0))}\n')
    else:
        m.write(f'/gps/direction {math.sin(math.radians(33.2))} 0.0 {math.cos(math.radians(33.2))}\n')  # aerogel limit
        # m.write(f'/gps/direction {math.sin(math.radians(35))} 0.0 {math.cos(math.radians(35))}\n') # gas limit
    m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 4:
    numRad = 4  # number of radial steps
    m.write(f'\n# radial scan test\n')
    if (runType == "vis"):
        m.write(f'/vis/scene/endOfEventAction accumulate\n')
        m.write(f'/vis/scene/endOfRunAction accumulate\n')
    for r in list(linspace(rMin, rMax, numRad)):
        m.write(f'/gps/direction {r} 0.0 {zDirection * zMax}\n')
        m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 5:
    numRad = 3  # number of radial steps
    numPhi = 24  # number of phi steps, prefer even multiple of 6 (12,24,36) to check sector boundaries
    m.write(f'\n# azimuthal+radial scan test\n')
    if (runType == "vis"):
        m.write(f'/vis/scene/endOfEventAction accumulate\n')
        m.write(f'/vis/scene/endOfRunAction accumulate\n')
    for r in list(linspace(rMin, rMax, numRad)):
        for phi in list(linspace(0, 2 * math.pi, numPhi, endpoint=False)):
            if (phi > math.pi / 6 and phi < (2 * math.pi - math.pi / 6)): continue  # restrict to one sector
            x = r * math.cos(phi)
            y = r * math.sin(phi)
            m.write(f'/gps/direction {x} {y} {zDirection * zMax}\n')
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

elif testNum == 7:
    m.write(f'\n# momentum scan\n')
    m.write(f'/gps/direction 0.25 0.0 {zDirection}\n')
    for en in list(linspace(1, 60, 10)):
        m.write(f'/gps/ene/mono {en} GeV\n')
        m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 8:
    numTheta = 6  # number of theta steps
    numPhi = 24  # number of phi steps, prefer even multiple of 6 (12,24,36) to check sector boundaries
    m.write(f'\n# demonstrate rings\n')
    if (runType == "vis"):
        m.write(f'/vis/scene/endOfEventAction accumulate\n')
        m.write(f'/vis/scene/endOfRunAction accumulate\n')
    if (zDirection < 0):
        etaMin_ = 1.6
        etaMax_ = 3.8
    else:
        etaMin_ = 1.5
        etaMax_ = 3.4
    thetaMin_ = 2 * math.atan(math.exp(-etaMax_))
    thetaMax_ = 2 * math.atan(math.exp(-etaMin_))
    print(f'SET theta range to {math.degrees(thetaMin_)} to {math.degrees(thetaMax_)} deg')
    for theta in list(linspace(thetaMin_, thetaMax_, numTheta)):
        for phi in list(linspace(0, 2 * math.pi, numPhi, endpoint=False)):
            if (phi > math.pi / 6 and phi < (2 * math.pi - math.pi / 6)): continue  # restrict to one sector
            if (abs(phi) > 0.001 and abs(theta - thetaMin_) < 0.001): continue  # allow only one ring at thetaMin
            x = math.sin(theta) * math.cos(phi)
            y = math.sin(theta) * math.sin(phi)
            z = math.cos(theta) * zDirection
            m.write(f'/gps/direction {x} {y} {z}\n')
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
    m.write(f'\n# opticalphoton parallel-to-point focusing\n')
    m.write(f'/vis/scene/endOfEventAction accumulate\n')
    m.write(f'/vis/scene/endOfRunAction accumulate\n')
    m.write(f'/gps/pos/type Beam\n')
    m.write(f'/gps/ang/type beam1d\n')
    for rVal in list(linspace(rMin, rMax, 5)):  # number of beams within theta acceptance
        m.write(f'/gps/ang/rot1 -{zMax} 0 {rVal}\n')
        m.write(f'/gps/pos/rot1 -{zMax} 0 {rVal}\n')
        m.write(f'/gps/pos/halfx 16 cm\n')  # parallel beam width
        m.write(f'/run/beamOn {numEvents}\n')

elif testNum == 13:
    m.write(f'\n# evenly distributed sensor hits test\n')
    if runType == "vis":
        m.write(f'/vis/scene/endOfEventAction accumulate\n')
        m.write(f'/vis/scene/endOfRunAction accumulate\n')
    if zDirection < 0:
        etaMin_ = 1.6
        etaMax_ = 3.8
    else:
        etaMin_ = 1.5
        etaMax_ = 3.4
    thetaMin_ = 2 * math.atan(math.exp(-etaMax_))
    thetaMax_ = 2 * math.atan(math.exp(-etaMin_))

    from scripts import createAngles
    theta_min = thetaMin_  # minimum polar angle
    theta_max = thetaMax_  # maximum polar angle
    num_rings = 12  # number of concentric rings, type=int
    hit_density = 20  # amount of photon hits for the smallest polar angle, type=int
    angles = createAngles.makeAngles(theta_min, theta_max, num_rings, hit_density)  # list of angles

    print(f'SET theta range to {math.degrees(thetaMin_)} to {math.degrees(thetaMax_)} deg')
    for angle in angles:
        theta, phi = angle[0], angle[1]
        if math.pi / 6 < phi < (2 * math.pi - math.pi / 6): continue  # restrict to one sector
        if abs(phi) > 0.001 and abs(theta - thetaMin_) < 0.001: continue  # allow only one ring at thetaMin
        x = math.sin(theta) * math.cos(phi)
        y = math.sin(theta) * math.sin(phi)
        z = math.cos(theta) * zDirection
        m.write(f'/gps/direction {x} {y} {z}\n')
        m.write(f'/run/beamOn {numEvents}\n')

elif testNum > 0:
    print("ERROR: unknown test number\n")
    m.close()
    sys.exit(2)

### finalize
if (runType == "vis"):
    m.write(f'/vis/viewer/flush\n')
    m.write(f'/vis/viewer/refresh\n')

### print macro and close stream
m.seek(0, 0)
if (testNum > 0):
    print(m.read())
m.close()

# RUN npsim
#########################################################

### simulation executable and arguments
cmd = "npsim"
cmd += " --runType " + runType
cmd += " --compactFile " + compactFile
# cmd += " --random.seed 1 "
cmd += " --outputFile " + outputFileName
if (testNum > 0):
    cmd += " --macro " + m.name
    cmd += " --enableG4GPS"
else:
    cmd += f' -N {numEvents}'
    cmd += " --inputFiles '" + inputFileName + "'"

### run simulation
print(sep)
print('EXECUTE: ' + cmd)
print(sep)
subprocess.call(shlex.split(cmd), cwd=detPath)

### cleanup
# os.remove(m.name) # remove macro
print("\nPRODUCED SIMULATION OUTPUT FILE: " + outputFileName)
