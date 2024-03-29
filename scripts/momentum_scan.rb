#!/usr/bin/env ruby
# run momentum scan test for various particles

require 'open3'
require 'fileutils'
require 'pycall/import'

## SETTINGS ########################################
NumEvents         = 70    # number of events per fixed momentum (full test: 50)
NumPoints         = 10    # number of momenta to sample (full test: 100)
PoolSize          = 6     # number of parallel threads to run
RunSimulation     = true  # if true, run the simulation step
RunReconstruction = true  # if true, run the reconstruction step
UseRINDEXrange    = false # if true, use range of RINDEX values rather than a single reference value
MaxNphot          = 500   # maximum number of incident photons expected (for plot drawing range)
####################################################


## args
if ARGV.length<2
  $stderr.puts """
  USAGE: #{$0} [d/p] [j/e]
     [d/p]: detector
       d: dRICH
       p: pfRICH
     [j/e]: reconstruction
       j: juggler [DEPRECATED, do not use]
       e: eicrecon
  """
  exit 2
end

## detector-specific settings
## FIXME: :rIndexRef values are averages? 
case ARGV[0]
when "d"
  zDirection = 1
  xRICH      = "dRICH"
  xrich      = "drich"
  radiator_h = {
    :agl => { :id=>0, :testNum=>7, :rIndexRef=>1.0190,  :rIndexRange=>[1.01852,1.02381], :maxMomentum=>20.0, :maxNPE=>20, },
    :gas => { :id=>1, :testNum=>8, :rIndexRef=>1.00076, :rIndexRange=>[1.00075,1.00084], :maxMomentum=>60.0, :maxNPE=>40, },
  }
when "p"
  zDirection = -1
  xRICH      = "pfRICH"
  xrich      = "pfrich"
  radiator_h = {
    :agl => { :id=>0, :testNum=>7, :rIndexRef=>1.0190, :rIndexRange=>[1.01852,1.02381], :maxMomentum=>20.0, :maxNPE=>40, },
    :gas => { :id=>1, :testNum=>8, :rIndexRef=>1.0013, :rIndexRange=>[1.0013,1.0015],   :maxMomentum=>60.0, :maxNPE=>40, },
  }
else
  $stderr.puts "ERROR: unknown detector '#{ARGV[0]}'"
  exit 1
end

## reconstruction specific settings
case ARGV[1]
when "j"
  reconMethod      = :juggler
when "e"
  reconMethod      = :eicrecon
else
  $stderr.puts "ERROR: unknown reconstruction '#{ARGV[1]}'"
  exit 1
end

## list of particles to test
particle_h = {
  'e-'          => { :mass=>0.00051, },
  'pi+'         => { :mass=>0.13957, },
  'kaon+'       => { :mass=>0.49368, },
  'proton'      => { :mass=>0.93827, },
  # 'e+'          => { :mass=>0.00051, },
  # 'pi-'         => { :mass=>0.13957, },
  # 'kaon-'       => { :mass=>0.49368, },
  # 'anti_proton' => { :mass=>0.93827, },
}

## produce output file dir and names
OutputDir = "out/momentum_scan.#{xrich}" # output directory ( ! will be overwritten ! )
if RunSimulation
  FileUtils.rm_r OutputDir, secure: true, verbose: true, force: true
  FileUtils.mkdir_p OutputDir
end
def out_file(prefix,ext)
  "#{OutputDir}/#{prefix}.#{ext}"
end

## run momentum scan simulation for each particle in `particle_h`
particle_h.keys.product(radiator_h.keys).each_slice(PoolSize) do |slice|
  pool = slice.map do |particle,rad_name|
    rad = radiator_h[rad_name]
    # preparation
    sim_file = out_file particle, "sim.#{rad_name}.root"
    rec_file = out_file particle, "rec.#{rad_name}.root"
    ana_file = rec_file.sub /\.root/, '.ana.root'
    cmds = []
    # simulation
    if RunSimulation
      cmds << [
        './simulate.py',
        "-t#{rad[:testNum]}",
        "-d#{zDirection}",
        "-p#{particle}",
        "-n#{NumEvents}",
        "-k#{NumPoints}",
        "-o#{sim_file}",
      ]
    end
    # reconstruction
    if RunReconstruction
      case reconMethod
      when :juggler
        cmds << [
          './juggler.sh',
          "-#{xrich[0]}",
          "-i#{sim_file}",
          "-o#{rec_file}",
        ]
      when :eicrecon
        cmds << [
          './benchmark.rb',
          '-r',
          "--sim-file #{sim_file}",
          "--rec-file #{rec_file}",
          "--ana-file #{ana_file}",
        ]
      end
    end
    # analysis
    plot_file = out_file particle, "rec_plots.#{rad_name}.root"
    cmds << [ 'root', '-b', '-q' ]
    case reconMethod
    when :juggler
      cmds.last << "scripts/src/momentum_scan_juggler_draw.C'(\"#{rec_file}\",\"#{plot_file}\",\"#{xrich.upcase}\",#{rad[:id]})'"
    when :eicrecon
      cmds.last << "scripts/src/momentum_scan_eicrecon_draw.C'(\"#{ana_file}\",\"#{plot_file}\",#{rad[:id]})'"
    end
    # spawn thread
    Thread.new do
      cmds.each_with_index do |cmd,i|
        cmd_shell = cmd.join ' '
        puts cmd_shell
        mode = i==0 ? 'w' : 'a'
        Open3.pipeline(
          cmd_shell,
          out: [out_file(particle,"#{rad_name}.log.out"),mode],
          err: [out_file(particle,"#{rad_name}.log.err"),mode],
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

# draw 2D hadd plots
if reconMethod == :eicrecon
  radiator_h.each do |rad_name,rad|
    # calculate n_group_rebin, for rebinning the momentum bins:
    # n_group_rebin => ceiling[ num momentum bins * (maxMomentum here) / (maxMomentum in histogram) * (1/NumPoints) ]
    # FIXME: automate the hard-coded numbers
    n_group_rebin = ( 500.0 * rad[:maxMomentum]/70.0 * 1.0/NumPoints ).to_i + 1
    drawArgs = [
      "\"#{OutputDir}/*.rec.#{rad_name}.ana.root\"",
      "\"#{OutputDir}/_theta_scan_2D.#{rad_name}\"",
      rad[:id],
      n_group_rebin
    ].join ','
    system "root -b -q scripts/src/momentum_scan_2D_draw.C'(#{drawArgs})'"
  end
end

# print errors for one of the particles
puts "ERRORS (for one job) ======================="
system "cat #{Dir.glob(OutputDir+"/*.err").first}"
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
  draw_h = particle_h.keys.map do |particle|
    [
      particle,
      {
        :root_file => r.TFile.new(out_file(particle, "rec_plots.#{rad_name}.root")),
        :color     => default_color,
        :marker    => default_marker,
      }.to_h
    ]
  end.to_h
  draw_h['e-'][:color]      = r.kBlack            if particle_h.has_key? 'e-'
  draw_h['pi+'][:color]     = r.kBlue             if particle_h.has_key? 'pi+'
  draw_h['kaon+'][:color]   = r.kGreen+1          if particle_h.has_key? 'kaon+'
  draw_h['proton'][:color]  = r.kMagenta          if particle_h.has_key? 'proton'
  draw_h['e-'][:marker]     = r.kFullCircle       if particle_h.has_key? 'e-'
  draw_h['pi+'][:marker]    = r.kFullSquare       if particle_h.has_key? 'pi+'
  draw_h['kaon+'][:marker]  = r.kFullTriangleUp   if particle_h.has_key? 'kaon+'
  draw_h['proton'][:marker] = r.kFullTriangleDown if particle_h.has_key? 'proton'

  ## loop over plots (loop through the first particle's file, assume the rest have the same)
  first_root_file = draw_h[particle_h.keys.first][:root_file]
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

    ## loop over particles
    rad[:maxTheta] = 0
    draw_h.each do |particle,h|

      ## get the plot
      plot = h[:root_file].Get plot_name

      ## variables for this particle and radiator
      mass   = particle_h[particle][:mass]
      rindex = rad[:rIndexRef]

      ## minimum momentum for Cherenkov emission
      def calculate_mom_min(m,n)
        m / Math.sqrt(n**2-1)
      end
      mom_min = calculate_mom_min mass, rindex

      ## drop points with momentum < mom_min
      (1..plot.GetNbinsX).each do |bn|
        mom = plot.GetBinCenter bn
        if mom < mom_min
          plot.SetBinContent bn, 0
          plot.SetBinError   bn, 0
          plot.SetBinEntries bn, 0
        end
      end

      ## saturation Cherenkov angle
      def calculate_max_theta(n)
        1000 * Math.acos(1/n)
      end
      max_theta = calculate_max_theta rindex
      rad[:maxTheta] = [rad[:maxTheta],max_theta].max

      ## add plot to canvas
      pad_plot.cd
      plot.SetMarkerStyle h[:marker]
      plot.SetMarkerColor h[:color]
      plot.SetLineColor   h[:color]
      plot.SetMarkerSize  1.5
      plot.Draw 'SAME E X0'
      plot.GetYaxis.SetTitle 'Average ' + plot.GetTitle.sub(/ vs\..*/,'')
      plot.SetTitle          'Average ' + plot.GetTitle
      leg_hits.AddEntry plot, particle, 'PE'

      ## expected Cherenkov angle curve
      if plot_name.match?(/^theta_/)
        rindex_list = UseRINDEXrange ? rad[:rIndexRange] : [rindex]
        rindex_list.each_with_index do |n,i| 
          ftn_theta = r.TF1.new(
            "ftn_theta_#{particle}_#{rad_name}_#{i}",
            "1000*TMath::ACos(TMath::Sqrt(x^2+#{mass}^2)/(#{n}*x))",
            calculate_mom_min(mass,n),
            plot.GetXaxis.GetXmax,
          )
          ftn_theta.SetLineColor h[:color]
          ftn_theta.SetLineWidth 2
          ftn_theta.Draw 'SAME'
          # set plot maximum
          rad[:maxTheta] = [rad[:maxTheta],calculate_max_theta(n)].max # max expected
          rad[:maxTheta] = [rad[:maxTheta],plot.GetMaximum].max        # max point
        end
      end

      # set plot ranges
      plot.GetXaxis.SetRangeUser 0, 1.1*rad[:maxMomentum]
      case plot_name
      when /^thetaResid_/
        plot.GetYaxis.SetRangeUser -40, 40
      when /^theta_/
        plot.GetYaxis.SetRangeUser 0, 1.1*rad[:maxTheta]
      when /^npe_/
        plot.GetYaxis.SetRangeUser 0, rad[:maxNPE]
      when /^nphot_/
        plot.GetYaxis.SetRangeUser 0, MaxNphot
      end

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
