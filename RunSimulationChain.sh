#!/bin/bash
#drich-hepmc-writer.C("drich-hepmc.hepmc",500,1.5,50,211)'
# npsim --compactFile=./epic/epic_drich_only.xml --runType=run  -G -N=500 --inputFiles ./drich-hepmc.hepmc --outputFile=./out/irt.edm4hep.root --part.userParticleHandler='' --random.seed 0x12345678 --random.enableEventSeed

if [ $# -ne 6 ]; then
 echo "Please specify relevant commands: "
 echo "RunSimulationChain.sh startMom finalMom step eta nEvents partType"
 exit 1
fi
echo "We are running the Full Simulation"
echo " "
echo "The detector path is : $DETECTOR_PATH"
echo " "
myCompact=$DETECTOR_PATH"/epic_drich_only.xml"
echo "The Compact File is $myCompact"

mkdir -p dHepmcFiles
mkdir -p dSimFiles
mkdir -p dRecFiles

sMomentum=$1 
fMomentum=$2
sSize=$3
eta=$4
nEvents=$5
partType=$6

echo "HepMC file inputs-->"
echo "Intial Momentum : $sMomentum , Final Momentum: $fMomentum Step Size: $sSize"
echo "Eta: $eta , nEvents: $nEvents , Particle type: $partType"
for (( i=$sMomentum; i<=fMomentum; i=i+$sSize)); 
do
  fileName=dRICH.p.$i.eta.$eta.ev.$nEvents.part.$partType
  echo $fileName 
  hepmcDestination="./dHepmcFiles/"$fileName".hepmc"
  simDestination="./dSimFiles/"$fileName".edm4hep.root"
  recDestination="./dRecFiles/"$fileName".root"
  
  root -l -b -q "drich-hepmc-writer.C(\"$hepmcDestination\","$nEvents","$eta","$i","$partType")"
  
  npsim --compactFile=$myCompact --runType=run  -G -N=$nEvents --inputFiles $hepmcDestination --outputFile=$simDestination --part.userParticleHandler='' --random.seed 0x12345678 --random.enableEventSeed
 
  # Running Juggler
  bash recon.sh -j $simDestination $recDestination
done

