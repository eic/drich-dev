//
// root -l 'erich-hepmc-writer.cxx("out.hepmc", 100)'
//

#include "HepMC3/GenEvent.h"
#include "HepMC3/ReaderAscii.h"
#include "HepMC3/WriterAscii.h"
#include "HepMC3/Print.h"

#include <iostream>
#include <random>
#include <cmath>
#include <math.h>
#include <TMath.h>

using namespace HepMC3;

/** Generate single muon event with fixed three momentum **/
void drich_hepmc_writer(const char* out_fname, int n_events, float et, float pp, int pdgid)
{
  auto *DatabasePDG = new TDatabasePDG();
  int pdg = pdgid;
  auto *particle = DatabasePDG->GetParticle(pdg);

  WriterAscii hepmc_output(out_fname);
  int events_parsed = 0;
  GenEvent evt(Units::GEV, Units::MM);

  //std::random_device rd;
  unsigned int seed = 0x12345678;//(unsigned int)abs(rd());
  std::cout << "init seed for random generator is " << seed << std::endl;
  // Random number generator
  TRandom *rdmn_gen = new TRandom(seed);

  for (events_parsed = 0; events_parsed < n_events; events_parsed++) {
    GenParticlePtr p1 =
        std::make_shared<GenParticle>(FourVector(0.0, 0.0, 12.0, 12.0), 11, 4);
    GenParticlePtr p2 = std::make_shared<GenParticle>(
        FourVector(0.0, 0.0, 100.0, 100.004), 2212, 4); 

    GenVertexPtr v1 = std::make_shared<GenVertex>();
    v1->add_particle_in(p1);
   v1->add_particle_in(p2);

    // type 1 is final state;  
    float etmin    = et; float etmax=et + 0.01;
    Double_t eta   = rdmn_gen->Uniform(etmin, etmax);
    Double_t th    = 2*std::atan(exp(-eta));
    float pmin = pp; float pmax = pmin+0.1;
    Double_t p     = rdmn_gen->Uniform(pmin, pmax);
    Double_t phi   = rdmn_gen->Uniform(0.0, 2*M_PI);
    
    Double_t px    = p * std::cos(phi) * std::sin(th);
    Double_t py    = p * std::sin(phi) * std::sin(th);
    Double_t pz    = p * std::cos(th);
    
    GenParticlePtr pq = std::make_shared<GenParticle>(FourVector(
								 px, py, pz,
								 sqrt(p*p + pow(particle->Mass(), 2))),
						      pdg, 1);
    v1->add_particle_out(pq);
    evt.add_vertex(v1);

    if (events_parsed == 0) {
      std::cout << "First event: " << std::endl;
      Print::listing(evt);
    }

    hepmc_output.write_event(evt);
    if (events_parsed % 10000 == 0) {
      std::cout << "Event: " << events_parsed << std::endl;
    }
    evt.clear();
  }
  hepmc_output.close();
  std::cout << "Events parsed and written: " << events_parsed << std::endl;
  exit(0);
}

