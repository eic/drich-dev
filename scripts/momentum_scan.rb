#!/usr/bin/env ruby
# run momentum scan test for various particles

require 'pry'
require 'open3'
require 'fileutils'
require 'pycall/import'

## settings
TestNum   = 7                   # test number for simulate.py
NumEvents = 300                 # number of events per fixed momentum
PoolSize  = 4                   # number of parallel threads to run
OutputDir = 'out/momentum_scan' # output directory ( ! will be overwritten ! )
DrawOnly  = false               # if true, do not run simulation, only draw the result
MaxCounts = 320                 # vertical axis limit of the counts vs. momentum plot

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

## produce output file dir and names
unless DrawOnly
  FileUtils.rm_r OutputDir, secure: true, verbose: true, force: true
  FileUtils.mkdir_p OutputDir
end
def out_file(prefix,ext)
  "#{OutputDir}/#{prefix}.#{ext}"
end

## run momentum scan simulation for each particle in `particle_a`
unless DrawOnly
  particle_a.each_slice(PoolSize) do |slice|
    pool = slice.map do |particle|
      # simulation command
      cmds = []
      cmds << [
        './simulate.py',
        "-t#{TestNum}",
        '-s',
        "-p#{particle}",
        "-n#{NumEvents}",
        "-o#{out_file particle, 'root'}",
      ]
      cmds << [
        'bin/draw_hits',
        out_file(particle,'root'),
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
end

## start ROOT analysis
# - must be done after simulations, otherwise this script hangs
r = PyCall.import_module 'ROOT'
r.gROOT.SetBatch true
r.gStyle.SetOptStat 0
r.gStyle.SetLegendTextSize 0.1

## draw settings
default_color = r.kBlack
default_marker = r.kFullCircle
particle_h = particle_a.map do |particle|
  [
    particle,
    {
      :root_file => r.TFile.new(out_file(particle,'plots.root')),
      :color     => default_color,
      :marker    => default_marker,
    }.to_h
  ]
end.to_h
particle_h['e-'][:color]      = r.kBlack
particle_h['pi+'][:color]     = r.kBlue
particle_h['kaon+'][:color]   = r.kGreen+1
particle_h['proton'][:color]  = r.kMagenta
particle_h['e-'][:marker]     = r.kFullCircle
particle_h['pi+'][:marker]    = r.kFullSquare
particle_h['kaon+'][:marker]  = r.kFullTriangleUp
particle_h['proton'][:marker] = r.kFullTriangleDown

## draw
canv_hits = r.TCanvas.new 'canv_hits', 'canv_hits', 1000, 800
leg_hits = r.TLegend.new 0, 0.7, 1, 1
canv_hits.Divide 2,1
pad_plot = r.TPad.new 'pad_plot', 'pad_plot', 0,    0, 0.75, 1
pad_leg  = r.TPad.new 'pad_leg',  'pad_leg',  0.75, 0, 1,    1
pad_plot.SetGrid 1,1
pad_plot.Draw
pad_leg.Draw
particle_h.each do |particle,h|
  pad_plot.cd
  plot = h[:root_file].Get 'aveHitsVsP'
  plot.SetMarkerStyle h[:marker]
  plot.SetMarkerColor h[:color]
  plot.SetLineColor   h[:color]
  plot.SetMarkerSize  1.5
  plot.GetYaxis.SetRangeUser 0, MaxCounts
  plot.Draw 'SAME E X0'
  plot.GetXaxis.SetTitle 'Thrown momentum [GeV]'
  plot.GetYaxis.SetTitle 'Average number of hits'
  plot.SetTitle 'Average ' + plot.GetTitle
  leg_hits.AddEntry plot, particle, 'PE'
end
pad_leg.cd
leg_hits.Draw
canv_hits.SaveAs out_file('_ave_hits_vs_p','png')
particle_h.values.each{ |h| h[:root_file].Close }
