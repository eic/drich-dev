#!/usr/bin/env ruby
#--------------------------------------------------------------------#
# vary parameters of dRICH geometry, and run simulations in parallel #
# Author: C. Dilks                                                   #
#--------------------------------------------------------------------#

require 'awesome_print'
require 'nokogiri'
require 'fileutils'
require 'open3'
require 'timeout'
require 'pry'

### environment check
if ENV['DETECTOR'].nil? or ENV['DETECTOR_PATH'].nil?
  $stderr.puts "ERROR: source environ.sh"
  exit 1
end

### GLOBAL SETTINGS **********************************
Detector     = ENV['DETECTOR']                     # detector name
DetectorPath = ENV['DETECTOR_PATH']                # detector installation prefix path
CompactFile  = "#{DetectorPath}/compact/drich.xml" # dRICH compact file
# ***
Cleanup       = false                  # if true, remove transient files
MultiThreaded = true                   # if true, run one simulation job per thread
PoolSize      = [`nproc`.to_i-2,1].max # number of parallel threads to run (`if MultiThreaded`)
TimeLimit     = 300                    # terminate a pipeline if it takes longer than `TimeLimit` seconds (set to `0` to disable)


### ARGUMENTS ****************************************
OutputDirMain = 'out'
VariatorDir   = 'ruby/variator'
variator_code = 'template'
if ARGV.length<1
  $stderr.puts """
  USAGE: #{$0} [OUTPUT ID] [VARIATOR CODE (default=#{variator_code})]

    [OUTPUT ID] should be a name for this simulation run
    - output files will be written to #{OutputDirMain}/[OUTPUT ID]
    - warning: this directory will be *removed* before running jobs

    [VARIATOR CODE] is the file containing the variation code
    - default is '#{variator_code}', which is short-hand for '#{VariatorDir}/#{variator_code}.rb'
    - two options for specifying the file:
      - basename of a file in '#{VariatorDir}', e.g., '#{variator_code}'
      - path to a specific file, e.g., './my_personal_variations/var1.rb'
    - see examples and template.rb in '#{VariatorDir}' to help define your own
  """
  exit 2
end
OutputDir = [OutputDirMain,ARGV[0]].join '/'
variator_code = ARGV[1] if ARGV.length>1


### PREPARATION **************************************

# status printout
def print_status(message)
  puts "[***] #{message}"
end
print_status 'preparation'

# get units, given string with value # todo: might not work for every case; need to generalize
def get_units(str)
  if str.include?('*')
    '*' + str.split('*').last
  else
    ''
  end
end

# load variator code
if variator_code.include? '/' # if variator_code is a path to a file
  unless variator_code.match? /^(\/|\.\/)/ # unless starts with '/' or './'
    variator_code = "./#{variator_code}"
  end
elsif File.file?(variator_code) or File.file?(variator_code+'.rb') # elsif local file in pwd
  variator_code = "./#{variator_code}"
else # else assume file is in VariatorDir
  variator_code = "./#{VariatorDir}/#{variator_code}"
end
print_status "loading variator from #{variator_code}"
unless File.file?(variator_code) or File.file?(variator_code+'.rb')
  $stderr.puts "ERROR: cannot find variation code #{variator_code}"
  exit 1
end
require variator_code
variator = Variator.new
puts "="*60

# error collection
errors = Array.new
error = Proc.new do |message|
  errors << message
  $stderr.puts message
end

# make output directories
puts "Writing output to #{OutputDir}"
FileUtils.mkdir_p OutputDir
FileUtils.rm_r OutputDir, secure: true, verbose: true
[ 'compact', 'config', 'sim', 'log', ].each do |subdir|
  FileUtils.mkdir_p "#{OutputDir}/#{subdir}"
end

# set xpaths for constant fixed_settings
variator.fixed_settings.each do |setting|
  if setting.has_key? :constant
    setting[:xpath] = "//constant[@name=\"#{setting[:constant]}\"]"
    setting[:attribute] = 'value'
  end
end

# parse compact file
# - write a copy to OutputDir, so it's easier to diff with the variants
#   (xml parsers tend to re-format the syntax)
xml = Nokogiri::XML File.open(CompactFile)
compact_drich_orig = [OutputDir,'compact',File.basename(CompactFile)].join '/'
puts "write parsed XML tree to #{compact_drich_orig}"
File.open(compact_drich_orig,'w') { |out| out.puts xml.to_xml }

# build array of variants, the results of the variation functions
# - for each variation in `variation`, add key `:variants`, pointing to its variant array
# - each variant array element is the following Hash:
#   {
#     :value     => the variant value together with its units
#     :xpath     => xml node xpath (copied from variation)
#     :attribute => attribute name (copied from variation)
#   }
variator.varied_settings.each do |var|
  # get node
  nodes = xml.xpath var[:xpath]
  if nodes.size == 0
    $stderr.puts "ERROR: cannot find node at xpath #{var[:xpath]}"
    exit 1
  elsif nodes.size > 1
    error.call "WARNING: more than one node for xpath '#{var[:xpath]}'" # todo: add support for this case
  end
  # get units
  val_str = nodes.first.attr var[:attribute]
  units = get_units val_str
  # fill variant_values array by calling the variation function Proc
  variant_values = var[:function].call *var[:args], var[:count]
  # fill variant array with Hashes
  var[:variants] = variant_values.map do |val|
    {
      :xpath     => var[:xpath],
      :attribute => var[:attribute],
      :value     => "#{val}#{units}",
      :label     => var[:label],
    }
  end
end

# take the product of all variant arrays
# - builds a list `variant_settings_list` of all the possible variable settings
# - each element is itself a list of `variant_settings`, for a particular variant
variant_arrays = variator.varied_settings.map{ |var| var[:variants] }
variant_settings_list = variant_arrays.first.product *variant_arrays[1..]
# binding.pry

# calculate derived settings
variant_settings_list.each do |variant_settings|
  variator.derived_settings.each do |derived_setting|
    # fill valHash with variant-specific settings (which have a label);
    # units are stripped away and values are assumed to be floats
    valHash = variant_settings
      .find_all{ |h| not h[:label].nil? }
      .map{ |h| [ h[:label], h[:value].split('*').first.to_f ] }
      .to_h
    # get units for the derived setting
    nodes   = xml.xpath derived_setting[:xpath]
    val_str = nodes.first.attr derived_setting[:attribute]
    units   = get_units val_str
    # calculate derived settings value, and add the setting to `variant_settings`
    derived_value = derived_setting[:derivation].call(valHash)
    # add to `variant_settings`, including appended units
    variant_settings << { :value => "#{derived_value}#{units}" }.merge(derived_setting)
  end
end
# binding.pry


### PRODUCE COMPACT FILES **************************************
print_status 'loop over variants'
cleanup_list = []
simulations  = []
variant_settings_list.each_with_index do |variant_settings,variant_id|

  # clone the xml tree
  xml_clone = xml.dup

  # in `xml_clone`, set each attribute of this variant's settings, along with the fixed settings
  settings = variant_settings + variator.fixed_settings
  print_status "-----> setting variant #{variant_id}:"
  ap settings
  settings.each do |var|
    node = xml_clone.at_xpath var[:xpath]
    node.set_attribute var[:attribute], var[:value]
  end

  # create drich compact file variant `compact_drich`, by writing `xml_clone`
  # - this is a modification of `CompactFile`, with this variant's attributes set
  # - `compact_drich` is written to `#{DetectorPath}/compact`, and copied to `OutputDir`
  basename      = "#{File.basename(CompactFile,'.xml')}_variant#{variant_id}"
  compact_drich = "#{File.dirname(CompactFile)}/#{basename}.xml"
  print_status "produce compact file variant #{compact_drich}"
  File.open(compact_drich,'w') { |out| out.puts xml_clone.to_xml }
  FileUtils.cp compact_drich, "#{OutputDir}/compact"
  cleanup_list << compact_drich
  cleanup_list << "#{compact_drich}.bak"

  # create detector template config
  # - this will be combined with `compact_drich` to render the full detector compact file
  config_drich = "#{OutputDir}/config/#{basename}.yml"
  print_status "produce jinja2 config #{config_drich}"
  File.open(config_drich,'w') do |out|
    out.puts <<~EOF
      features:
        pid:
          drich: #{compact_drich}
    EOF
  end

  # render the full detector compact file, `compact_detector`
  # - it will include `compact_drich` instead of the default `CompactFile`
  compact_detector = "#{DetectorPath}/#{Detector}_#{basename}.xml"
  print_status "jinja2 render template to #{compact_detector}"
  render = [
    "#{Detector}/bin/make_detector_configuration",
    "-d #{Detector}/templates",
    "-t #{Detector}.xml.jinja2",
    "-o #{compact_detector}",
    "-c #{config_drich}",
  ]
  system render.join(' ')
  cleanup_list << compact_detector

  # simulation settings
  # NOTE: if you change this, update ruby/variator/template.md
  simulation_settings = {
    :id               => variant_id,
    :variant_info     => settings,
    :compact_detector => compact_detector,
    :compact_drich    => compact_drich,
    :output           => "#{OutputDir}/sim/#{basename}.root",
    :log              => "#{OutputDir}/log/#{basename}",
  }

  # build simulation pipeline command
  simulations << {
    :pipelines => variator.simulation_pipelines.call(simulation_settings)
  }.merge(simulation_settings)

end


### EXECUTION **************************************************

# run the commands listed in `sim[:pipelines]`, and log to `sim[:log]`
def execute_thread(sim)
  # status update
  print_thread_status = Proc.new do |message|
    puts "-> variant #{sim[:id]} -> #{message}"
  end
  print_thread_status.call "BEGIN"
  # print settings for this variant to log file
  File.open("#{sim[:log]}.info",'w') do |out|
    out.puts "VARIANT #{sim[:id]}:"
    out.write sim[:variant_info].ai(plain: true)
    out.puts "\n"
    out.puts "PIPELINE:"
    out.puts sim[:pipelines].map{ |p| p.join(' ') }.ai(plain: true)
  end
  # loop over pipelines
  timed_out = false
  sim[:pipelines].each do |simulation_pipeline|
    # execute pipeline, with logging, and timeout control
    print_thread_status.call simulation_pipeline.map(&:first).join(' | ')
    pipeline_waiters = []
    begin
      Timeout::timeout(TimeLimit) do
        # use `pipeline_start`, so calling thread is in control (allows Timeout::timeout to work)
        pipeline_waiters = Open3.pipeline_start(
          *simulation_pipeline,
          :out=>["#{sim[:log]}.out",'a'],
          :err=>["#{sim[:log]}.err",'a'],
        )
        Process.waitall # wait for all pipeline_waiters to finish
      end
    rescue Timeout::Error
      timed_out = true
      # print timeout error
      print_thread_status.call "TIMEOUT: #{simulation_pipeline.map(&:first).join(' | ')}"
      File.open("#{sim[:log]}.err",'a') do |out|
        out.puts '='*30
        out.puts "TIMEOUT LIMIT REACHED, terminate pipeline:"
        out.puts simulation_pipeline.join(' ')
        out.puts '='*30
      end
      # kill the timed-out pipeline
      pipeline_waiters.each do |waiter|
        print_thread_status.call "KILL #{waiter}"
        begin
          Process.kill('KILL',waiter.pid)
        rescue Errno::ESRCH
        end
      end
    end
    return if timed_out # do not run the next pipeline, if timed out
  end
  print_thread_status.call "END"
end

# execute the threads, either single- or multi-threaded
# - todo: use concurrency instead of fixed pool sizes (current implementation
#         is only efficient if all threads take the same time to run)
print_status 'SIMULATION COMMAND (for one variant):'
ap simulations.first
print_status 'begin simulation '.upcase + '='*40
if MultiThreaded
  print_status "running multi-threaded with PoolSize = #{PoolSize}"
  print_status "all pipelines have TimeLimit = #{TimeLimit} seconds"
  simulations.each_slice(PoolSize) do |slice|
    pool = slice.map do |simulation|
      Thread.new{ execute_thread simulation }
    end
    trap 'INT' do
      print_status 'interrupt received; killing threads...'
      pool.each &:kill
      exit 1
    end
    pool.each &:join
  end
else
  simulations.each do |simulation|
    execute_thread simulation
  end
end

# cleanup the transient compact files
if Cleanup
  print_status "cleanup transient files:"
  ap cleanup_list.sort
  cleanup_list.each do |file|
    FileUtils.rm_f file, verbose: true
  end
end

# collect and print
print_status 'DONE'
simulations.each do |simulation|
  err_log = simulation[:log]+".err"
  num_errors = `grep -v '^$' #{err_log} | wc -l`.chomp.split.first.to_i
  if num_errors>0
    errors << "  #{err_log}  => #{num_errors} errors"
  end
end
if errors.size>0
  print_status 'ERRORS:'
  ap errors
else
  print_status 'NO ERRORS'
end
