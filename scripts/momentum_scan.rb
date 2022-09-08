#!/usr/bin/env ruby
# run momentum scan test for various particles

require 'pry'
require 'open3'
require 'fileutils'
require 'pycall/import'

## settings
NumEvents = 100                 # number of events per fixed momentum
NumPoints = 10                  # number of momenta to sample
PoolSize  = 6                   # number of parallel threads to run
OutputDir = 'out/momentum_scan' # output directory ( ! will be overwritten ! )
RunSimRec = false               # if false, do not run simulation+reconstruction, only draw the result

## list of particles to test
particle_a = [
  'e-',
  'pi+',
  'kaon+',
  'proton',
  # 'e+',
  # 'pi-',
  # 'kaon-',
  # 'anti_proton',
]

## radiators
radiator_h = {
  :agl => { :id=>0, :testNum=>7, },
  :gas => { :id=>1, :testNum=>8, },
}

## warn if not doing simulation
puts "\nNOTE: skipping simulation+reconstruction, since RunSimRec=false\n\n" unless RunSimRec

## produce output file dir and names
if RunSimRec
  FileUtils.rm_r OutputDir, secure: true, verbose: true, force: true
  FileUtils.mkdir_p OutputDir
end
def out_file(prefix,ext)
  "#{OutputDir}/#{prefix}.#{ext}"
end

## run momentum scan simulation for each particle in `particle_a`
particle_a.product(radiator_h.keys).each_slice(PoolSize) do |slice|
  pool = slice.map do |particle,rad_name|
    rad = radiator_h[rad_name]
    # preparation
    sim_file = out_file particle, "sim.#{rad_name}.root"
    rec_file = out_file particle, "rec.#{rad_name}.root"
    cmds = []
    # simulation + reconstruction
    if RunSimRec
      cmds << [
        './simulate.py',
        "-t#{rad[:testNum]}",
        '-s',
        "-p#{particle}",
        "-n#{NumEvents}",
        "-k#{NumPoints}",
        "-o#{sim_file}",
      ]
      cmds << [
        './recon.sh',
        "-j",
        "-i #{sim_file}",
        "-o #{rec_file}",
      ]
    end
    # analysis
    plot_file = out_file particle, "rec_plots.#{rad_name}.root"
    cmds << [
      'root', '-b', '-q',
      "scripts/src/momentum_scan_draw.C(\"#{rec_file}\",\"#{plot_file}\",#{rad[:id]})"
    ]
    # spawn thread
    Thread.new do
      cmds.each_with_index do |cmd,i|
        puts cmd.join ' '
        mode = i==0 ? 'w' : 'a'
        Open3.pipeline(
          cmd,
          out: [out_file(particle,'log.out'),mode],
          err: [out_file(particle,'log.err'),mode],
        )
      end
    end
  end
  trap 'INT' do
    pool.each &:kill
    exit 1
  end
  # wait for pool to finish
  pool.each &:join
end

# print errors for one of the particles
puts "ERRORS (for one particle) ======================="
system "cat #{out_file particle_a.first, 'log.err'}"
puts "END ERRORS ======================================"


#############################################################


## start ROOT analysis
# - must be done after simulations, otherwise this script hangs
r = PyCall.import_module 'ROOT'
r.gROOT.SetBatch true
r.gStyle.SetOptStat 0
r.gStyle.SetLegendTextSize 0.1

## loop over radiators
radiator_h.each do |rad_name,rad|

  ## draw settings
  default_color  = r.kBlack
  default_marker = r.kFullCircle
  draw_h = particle_a.map do |particle|
    [
      particle,
      {
        :root_file => r.TFile.new(out_file(particle, "rec_plots.#{rad_name}.root")),
        :color     => default_color,
        :marker    => default_marker,
      }.to_h
    ]
  end.to_h
  draw_h['e-'][:color]      = r.kBlack
  draw_h['pi+'][:color]     = r.kBlue
  draw_h['kaon+'][:color]   = r.kGreen+1
  draw_h['proton'][:color]  = r.kMagenta
  draw_h['e-'][:marker]     = r.kFullCircle
  draw_h['pi+'][:marker]    = r.kFullSquare
  draw_h['kaon+'][:marker]  = r.kFullTriangleUp
  draw_h['proton'][:marker] = r.kFullTriangleDown

  ## loop over plots (loop through the first particle's file, assume the rest have the same)
  first_root_file = draw_h[particle_a.first][:root_file]
  PyCall.iterable(first_root_file.GetListOfKeys).each do |tkey|
    plot_name = tkey.GetName
    next unless plot_name.match? /scan_pfx$/
    
    ## canvas
    canv = r.TCanvas.new "#{plot_name}_#{rad_name}_canv", "#{plot_name}_#{rad_name}_canv", 1000, 800
    leg_hits = r.TLegend.new 0, 0.7, 1, 1
    canv.Divide 2,1
    pad_plot = r.TPad.new 'pad_plot', 'pad_plot', 0,    0, 0.75, 1
    pad_leg  = r.TPad.new 'pad_leg',  'pad_leg',  0.75, 0, 1,    1
    pad_plot.SetGrid 1,1
    pad_plot.SetLeftMargin 0.2
    pad_plot.Draw
    pad_leg.Draw

    ## loop over particles, add plot to canvas
    draw_h.each do |particle,h|
      pad_plot.cd
      plot = h[:root_file].Get plot_name
      plot.SetMarkerStyle h[:marker]
      plot.SetMarkerColor h[:color]
      plot.SetLineColor   h[:color]
      plot.SetMarkerSize  1.5
      plot.Draw 'SAME E X0'
      plot.GetYaxis.SetTitle 'Average ' + plot.GetTitle.sub(/ vs\..*/,'')
      plot.SetTitle          'Average ' + plot.GetTitle
      leg_hits.AddEntry plot, particle, 'PE'
    end

    ## save canvas to file
    pad_leg.cd
    leg_hits.Draw
    if plot_name.match?(/^nphot_/) # :gas and :agl plots are the same, just save :gas
      canv.SaveAs out_file("_#{plot_name}",'png') if rad_name==:gas 
    else
      canv.SaveAs out_file("_#{plot_name}.#{rad_name}",'png')
    end

  end # loop over plots

  draw_h.values.each{ |h| h[:root_file].Close }

end # loop over radiators
