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
acceptanceDict = {
    'drich': {
        # theta limits [degrees]
        'thetaMin': 2.8,
        'thetaMax': 30.5,
    },
    'pfrich': {
        # theta limits [degrees]
        'thetaMin': 180.0 - 10.0, # FIXME
        'thetaMax': 180.0 - 70.0, # FIXME
    },
}

# ARGUMENTS
################################################################

inputFileName = ''
testNum = -1
standalone = False
compactFileCustom = ''
zDirection = 1
particle_name = 'pi+'
particle_momentum = 20.0 # [GeV]
particle_theta = 23.5 # [deg]
runType = 'run'
numEvents = 50
numTestSamples = 0
restrict_sector = True
outputImageType = ''
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
                14:   parallel-to-point focal test, beams over entire acceptance
                        ( recommend: optDbg=4 / mirDbg=0 / sensDbg=0)

[OPTIONAL ARGUMENTS]

    OPTIONS:    -d: direction to throw particles (may not be used by all tests)
                    1 = toward dRICH (default)
                   -1 = toward pfRICH
                -s: enable standalone RICH-only simulation (default is full detector)
                -c [compact file]: specify a custom compact file
                   (this will override -d and -s options)
                -p [particle]: name of particle to throw; default: {particle_name}
                   examples:
                    - e- / e+
                    - pi+ / pi-
                    - kaon+ / kaon-
                    - proton / anti_proton
                    - opticalphoton
                -m [momentum]: momentum (GeV) for mono-energetic runs (default={particle_momentum})
                -a [angle]: fixed polar angle for certain tests [deg] (default={particle_theta})
                -n [numEvents]: number of events to process (default={numEvents})
                   - if using TEST_NUM, this is usually the number of events PER fixed momentum
                   - if using INPUT_FILE, you can set to 0 to run ALL events in the file, otherwise
                     it will run the default amount of {numEvents}
                -k [numTestSamples]: some tests throw particles in multiple different directions,
                   such as "polar scan test"; for this test, use [numTestSamples] to control
                   how many directions are tested
                   - many tests offer a similar usage of [numTestSamples]
                   - these tests also have default [numTestSamples] values
                -l: allow azimuthal scans to cover the full 2*pi range, rather than restricting
                    to a single sector
                -r: run, instead of visualize (default)
                -v: visualize, instead of run
                   - it is HIGHLY recommended to set `DRICH_debug_sector` to `1` in `drich.xml`,
                     which will draw one sector and set visibility such that you can see inside
                     the dRICH
                   - standalone mode will be automatically enabled
                -e [output image extension]: save visual with specified type (svg,pdf,ps)
                   - useful tip: if you want to suppress the drawing of the visual, but
                     still save an output image, use Xvbf (start EIC container shell
                     as `xvfb-run eic-shell`); this is good for batch processing
                -o [output file]: output root file name (overrides any default name)
    '''

if (len(sys.argv) <= 1):
    print(helpStr)
    sys.exit(2)
try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:t:d:sc:p:m:a:n:k:lrve:o:')
except getopt.GetoptError:
    print('\n\nERROR: invalid argument\n', helpStr, file=sys.stderr)
    sys.exit(2)
for opt, arg in opts:
    if (opt == '-i'): inputFileName = arg.lstrip()
    if (opt == '-t'): testNum = int(arg)
    if (opt == '-d'): zDirection = int(arg)
    if (opt == '-s'): standalone = True
    if (opt == '-c'): compactFileCustom = arg.lstrip()
    if (opt == '-p'): particle_name = arg.lstrip()
    if (opt == '-m'): particle_momentum = float(arg)
    if (opt == '-a'): particle_theta = float(arg)
    if (opt == '-n'): numEvents = int(arg)
    if (opt == '-k'): numTestSamples = int(arg)
    if (opt == '-l'): restrict_sector = False
    if (opt == '-r'): runType = 'run'
    if (opt == '-v'): runType = 'vis'
    if (opt == '-e'): outputImageType = arg.lstrip()
    if (opt == '-o'): outputFileName = arg.lstrip()
if (testNum < 0 and inputFileName == ''):
    print('\n\nERROR: Please specify either an input file (`-i`) or a test number (`-t`).\n', helpStr, file=sys.stderr)
    sys.exit(2)
elif (testNum > 0 and inputFileName != ''):
    print('\n\nWARNING: You specified both an input file and a test number; proceeding with the input file only.\n', file=sys.stderr)
    testNum = -1

### overrides
if (testNum >= 10):
    print("optics test, overriding some settings...")
    particle_name = 'opticalphoton'
    standalone = True
    if (testNum in [10,11,12]):
        print("-- this is a visual test --")
        runType = 'vis'
if (particle_name == "opticalphoton"):
    particle_momentum = 3e-9
    print(f'optical photons test: using energy {particle_momentum}')
if runType == 'vis':
    standalone = True

### helper functions
# convert momentum -> kinetic energy
def momentum_to_kinetic_energy(p,part):
    # first get the mass
    mass = 0.0
    if bool(re.search('^e[+-]$',part)):
        mass = 0.000510999
    elif bool(re.search('^pi[+-]$',part)):
        mass = 0.13957
    elif bool(re.search('^kaon[+-]$',part)):
        mass = 0.493677
    elif bool(re.search('proton$',part)):
        mass = 0.938272
    elif (part == "opticalphoton"):
        mass = 0.0
    else:
        print(f'WARNING: mass for particle "{part}" needs to be added to simulate.py; assuming momentum==energy for now', file=sys.stderr)
    # then convert to energy
    en = math.sqrt( math.pow(p,2) + math.pow(mass,2) )
    kin_en = en - mass # total energy = kinetic energy + rest energy
    print(f'Momentum {p} GeV converted to Kinetic Energy {kin_en} GeV')
    return kin_en


### configure input and output file names
### relative paths will be made absolute here
workDir = os.getcwd()
##### ensure input file name has absolute path
if inputFileName != '':
    if not bool(re.search('^/', inputFileName)): inputFileName = workDir + "/" + inputFileName
##### ensure output file name has absolute path (and generate default name, if unspecified)
if outputFileName == '':
    outputFileName = workDir + "/out/sim.edm4hep.root"  # default name
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

detMain = os.environ['DETECTOR_CONFIG']
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
print(f'particle       = {particle_name}')
print(f'particle_theta = {particle_theta} deg')
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
m.write(f'/gps/particle {particle_name}\n')
m.write(f'/gps/number 1\n')

### convert momentum to energy, mono-energetic gun
if (testNum != 7 and testNum != 8):
    energy = momentum_to_kinetic_energy(particle_momentum,particle_name)
    m.write(f'/gps/ene/mono {energy} GeV\n')

### append source settings
m.write(f'/gps/position 0 0 0 cm\n')

# ACCEPTANCE LIMITS
################################################################

### set angular acceptance limits
thetaMin = math.radians(acceptanceDict[xrich]['thetaMin'])
thetaMax = math.radians(acceptanceDict[xrich]['thetaMax'])
def theta_to_eta(th):
    return -math.log(math.tan(0.5 * th))
etaMin = theta_to_eta(thetaMax)
etaMax = theta_to_eta(thetaMin)
print(sep)
print('** acceptance limits **')
print(f'thetaMin = {math.degrees(thetaMin)} deg')
print(f'thetaMax = {math.degrees(thetaMax)} deg')
print(f'etaMin = {etaMin}')
print(f'etaMax = {etaMax}')
print(sep)

evnum = 0 # event number counter (for logging)

# TEST SETTINGS
######################################

### `switch testNum:`

if testNum == 1:
    m.write(f'\n# aim at +x {xRICH} sector\n')
    x = math.sin(math.radians(particle_theta))
    y = 0.0
    z = math.cos(math.radians(particle_theta)) * zDirection
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
    m.write(f'\n# polar scan test\n')
    numTheta = 4 if numTestSamples==0 else numTestSamples  # number of theta steps
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
        for _ in range(numEvents):
            print(f'evnum = {evnum}   theta = {math.degrees(theta)} deg   eta = {theta_to_eta(theta)}')
            evnum += 1

elif testNum == 5:
    m.write(f'\n# polar+azimuthal scan test\n')
    numTheta = 4 if numTestSamples==0 else numTestSamples # number of theta steps
    numPhi = 24  # number of phi steps, prefer even multiple of 6 (12,24,36) to check sector boundaries
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
    m.write(f'\n# momentum scan\n')
    numMomPoints = 10 if numTestSamples==0 else numTestSamples # number of momenta
    x = math.sin(math.radians(particle_theta))
    y = 0.0
    z = math.cos(math.radians(particle_theta)) * zDirection
    m.write(f'/gps/direction {x} {y} {z}\n')
    momMax = 60
    if testNum == 7:
        momMax = 20
    for mom in list(linspace(1, momMax, numMomPoints)):
        en = momentum_to_kinetic_energy(mom,particle_name)
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
    numTheta = 5 if numTestSamples==0 else numTestSamples # number of theta steps
    m.write(f'\n# opticalphoton parallel-to-point focusing\n')
    m.write(f'/vis/scene/endOfEventAction accumulate\n')
    m.write(f'/vis/scene/endOfRunAction accumulate\n')
    m.write(f'/gps/pos/type Beam\n')
    m.write(f'/gps/ang/type beam1d\n')
    for theta in list(linspace(thetaMin, thetaMax, numTheta)):
        x = math.sin(theta)
        y = 0.0
        z = math.cos(theta) * zDirection
        m.write(f'/gps/ang/rot1 -{z} {y} {x}\n') # different coordinate system...
        m.write(f'/gps/pos/rot1 -{z} {y} {x}\n')
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
    print("ERROR: unknown test number\n", file=sys.stderr)
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
        f'--outputFile {outputFileName}',
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
        f'--inputFiles \'{inputFileName}\'',
        ])
    if (numEvents > 0):
        cmd.extend([ f'-N {numEvents}' ])
    else:
        cmd.extend([ f'-N -1' ])

### run simulation
cmdShell = shlex.split(" ".join(cmd))
print(f'{sep}\nRUN SIMULATION:\n{shlex.join(cmdShell)}\n{sep}')
subprocess.run(cmdShell, cwd=detPath)

### cleanup
# os.remove(m.name) # remove macro
print("\nPRODUCED SIMULATION OUTPUT FILE: " + outputFileName)
