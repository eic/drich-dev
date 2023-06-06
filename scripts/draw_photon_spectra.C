void draw_photon_spectra(
    TString sim_file_name = "out/sim.edm4hep.root",
    TString rec_file_name = "out/rec.edm4hep.root"
    )
{
  auto sim_file = TFile::Open(sim_file_name);
  auto rec_file = TFile::Open(rec_file_name);
  auto sim_tree = (TTree*) sim_file->Get("events");
  auto rec_tree = (TTree*) rec_file->Get("events");
  auto canv = new TCanvas("canv","canv",1600,1200);
  canv->Divide(2,2);
  for(int i=1; i<=4; i++)
    canv->GetPad(i)->SetGrid(1,1);
  canv->cd(1);
  sim_tree->Draw("1239.8/DRICHHits.EDep/1e9");
  canv->cd(2);
  rec_tree->Draw("1239.8/DRICHAerogelIrtCherenkovParticleID.photonEnergy/1e9");
  canv->cd(4);
  rec_tree->Draw("1239.8/DRICHGasIrtCherenkovParticleID.photonEnergy/1e9");
  canv->SaveAs("out/spectra.png");
  sim_file->Close();
  rec_file->Close();
}
