#!/usr/bin/env ruby
# make set of training data for GNNs
# - execute from top-level directory

require 'fileutils'
require 'awesome_print'
require 'thread/pool'

#### SETTINGS ################
nThreads = `nproc`.to_i-2 # number of parallel threads (if nproc fails, just hardcode a number)
outputDir = 'out/trainingSample'
hepmcFile = 'hepmc/pythia8NCDIS_10x100_minQ2=1_beamEffects_xAngle=-0.025_hiDiv_vtxfix_1_000.hepmc'
gunTests = {
  1 => 'single',
  5 => 'spray',
}
numEvents = {
  :forHepmc => 1000,
  :forGun   => 100,
}
particles = [
  'e-',
  'pi+',
  # 'pi-',
  'kaon+',
  # 'kaon-',
  'proton',
  # 'anti_proton',
]
energies = [
  1,
  2,
  4,
  8,
  16,
  30,
  40,
  50,
]
###########################

### setup
FileUtils.mkdir_p outputDir
cmds = []

### gun tests
gunTests.keys.product(particles,energies).each do |testNum,particle,energy|
  outputFile = "#{outputDir}/" + [
    gunTests[testNum],
    particle,
    "#{energy}GeV",
    'root',
  ].join('.')
  cmd = [
    './simulate.py',
    "-t#{testNum}",
    '-d1',
    '-s',
    "-p#{particle}",
    "-n#{numEvents[:forGun]}",
    "-e#{energy}",
    '-r',
    "-o#{outputFile}",
    "&& bin/draw_hits d #{outputFile}"
  ]
  cmds << cmd.join(' ')
end

### use hepmc
outputFile = "#{outputDir}/" + [
  File.basename(hepmcFile,'.hepmc'),
  'root',
].join('.')
cmd = [
  './simulate.py',
  "-i'#{hepmcFile}'",
  '-d1',
  '-s',
  "-n#{numEvents[:forHepmc]}",
  '-r',
  "-o#{outputFile}",
  "&& bin/draw_hits d #{outputFile}"
]
cmds << cmd.join(' ')

### execution
ap cmds
# exit
pool = Thread.pool(nThreads)
cmds.each{|cmd|pool.process{system cmd}} # multi-threaded
# cmds.each{|cmd|system cmd} # single-threaded
pool.shutdown
puts "DONE: FILES PRODUCED IN #{outputDir}"
