#!/usr/bin/env ruby
# run momentum scan test for various particles

require 'pry'
require 'open3'
require 'pycall/import'


TestNum   = 7   # test number for simulate.py
NumEvents = 10  # number of events per fixed momentum
PoolSize  = 4   # number of parallel threads to run

default_color = 7

particle_list = [
  { :name=>'e-',          :color=>default_color, },
  { :name=>'pi+',         :color=>default_color, },
  { :name=>'kaon+',       :color=>default_color, },
  { :name=>'proton',      :color=>default_color, },
  # { :name=>'e+',          :color=>r.kBlack, },
  # { :name=>'pi-',         :color=>r.kBlack, },
  # { :name=>'kaon-',       :color=>r.kBlack, },
  # { :name=>'anti_proton', :color=>r.kBlack, },
]

particle_list.each_slice(PoolSize) do |slice|
  pool = slice.map do |particle|
    out_file = "out/momentum_scan.#{particle[:name]}"
    cmds = []
    cmds << [
      './simulate.py',
      "-t#{TestNum}",
      '-s',
      "-p#{particle[:name]}",
      "-n#{NumEvents}",
      "-o#{out_file}.root",
    ]
    cmds << [ './drawHits.exe', out_file+'.root' ]
    Thread.new do
      cmds.each_with_index do |cmd,i|
        puts cmd.join ' '
        mode = i==0 ? 'w' : 'a'
        Open3.pipeline cmd, out: [out_file+'.out',mode], err: [out_file+'.err',mode]
      end
    end
  end
  trap 'INT' do
    pool.each &:kill
    exit 1
  end
  pool.each &:join
end

# start ROOT analysis
# - must be done after simulations, otherwise we get stuck in some mode
r = PyCall.import_module 'ROOT'
r.gROOT.SetBatch true
r.gStyle.SetOptStat 0
# default_color = r.kBlack
