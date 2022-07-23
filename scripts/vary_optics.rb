#!/usr/bin/env ruby
#--------------------------------------------------------------------#
# vary parameters of dRICH geometry, and run simulations in parallel #
# Author: C. Dilks                                                   #
#--------------------------------------------------------------------#

require 'numpy'
require 'awesome_print'
require 'nokogiri'
require 'fileutils'
require 'thread/pool'
require 'open3'
require 'pry'


### SETTINGS *************************************
Detector      = 'ecce'                          # path to detector repository
CompactFile   = "#{Detector}/compact/drich.xml" # compact file to vary
Cleanup       = false                           # if true, remove transient files from `#{Detector}/`
MultiThreaded = true                            # if true, run one simulation job per thread
PoolSize      = [`nproc`.to_i-2,1].max          # number of parallel threads to run (`if MultiThreaded`)


### SIMULATION COMMANDS **************************
# - list of commands to run the simulation
# - the full `simulation_pipelines` array is a list of pipelines, which will be
#   executed sequentially
#   - a pipeline is a list of commands, where stdout of one command is streamed
#     to stdin of the next command
#     - each command is written as an array, where the first element is the
#       command, and the remaining elements are its arguments
# - the list of pipelines will be executed for each variant
# - example pipelines:
#   [[ "ls", "-t" ]]                  # => `ls -t`
#   [ ["ls","-lt"], ["tail","-n3"] ]  # => `ls -lt | tail -n3`
simulation_pipelines = Proc.new do |compact_file,output_file|
  [
    [[
      "./simulate.py",
      "-t 1",
      "-c #{compact_file}",
      "-o #{output_file}",
    ]],
    # [
    #   ["exit"],
    #   [
    #     "echo",
    #     "./simulate.py",
    #     "-t12",
    #     "-v",
    #     "-msvg",
    #     "-c#{compact_file}",
    #     "-o#{output_file}",
    #   ],
    # ],
  ]
end


### VARIATION FUNCTIONS **************************
# - add any variation functions here
# - must return Array

# linearly vary by `center +/- delta`, `count` times
center_delta = Proc.new do |center, delta, count|
  Numpy.linspace(center-delta, center+delta, count).to_a
end

# linearly vary from `min` to `max`, `count` times
min_max = Proc.new do |min, max, count|
  Numpy.linspace(min, max, count).to_a
end


### PARAMETER VARIATIONS *************************
# create the following Hash for each variation, and
# add it to the Array `variations`:
#   {
#     :xpath     => XPATH to the XML node
#     :attribute => node attribute
#     :function => variation function (see above)
#     :args      => variation arguments
#     :count     => number of variations
#   }
variations = [
  {
    :xpath     => '//mirror',
    :attribute => 'focus_tune_x',
    :function  => center_delta,
    :args      => [70, 20],
    :count     => 4,
  },
  # {
  #   :xpath     => '//mirror',
  #   :attribute => 'focus_tune_y',
  #   :function  => min_max,
  #   :args      => [20, 50],
  #   :count     => 3,
  # },
]


### FIXED SETTINGS *******************************
# specify specific fixed settings, with similar Hashes, either of:
#   { :constant, :value }           # for `XPATH=//constant` nodes
#   { :xpath, :attribute, :value }  # for general attribute
fixed_settings = [
  # { :constant=>'DRICH_debug_optics', :value=>'1' },
]



#####################################################################
#
# BEGIN PROGRAM
#
#####################################################################


### ARGUMENTS ****************************************
OutputDirMain = "out"
if ARGV.length<1
  $stderr.puts """
  USAGE: #{$0} [OUTPUT ID]

    [OUTPUT ID] should be a name for this simulation run
    - output files will be written to #{OutputDirMain}/[OUTPUT ID]
    - warning: this directory will be *removed* before running jobs
  """
  exit 2
end
OutputDir = [OutputDirMain,ARGV[0]].join '/'

### PREPARATION **************************************

# status printout
def print_status(message)
  puts "[***] #{message}"
end
print_status 'preparation'

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
fixed_settings.each do |setting|
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
puts compact_drich_orig
File.open(compact_drich_orig,'w') { |out| out.puts xml.to_xml }

# build array of variants, the results of the variation functions
# - for each variation in `variation`, add key `:variants`, pointing to its variant array
# - each variant array element is the following Hash:
#   {
#     :value     => the variant value together with its units
#     :xpath     => xml node xpath (copied from variation)
#     :attribute => attribute name (copied from variation)
#   }
variations.each do |var|
  # get units
  nodes = xml.xpath var[:xpath]
  error.call "WARNING: more than one node for xpath '#{var[:xpath]}'" if nodes.size>1  # todo: add support for this case
  val_str = nodes.first.attr var[:attribute]
  units = val_str.include?('*') ?
    '*' + val_str.split('*').last :
    ''
  # fill variant_values array by calling the variation function Proc
  variant_values = var[:function].call *var[:args], var[:count]
  # fill variant array with Hashes
  var[:variants] = variant_values.map do |val|
    {
      :value     => "#{val}#{units}",
      :xpath     => var[:xpath],
      :attribute => var[:attribute],
    }
  end
end

# take the product of all variant arrays
# - builds a list `variant_settings_list` of all the possible variable settings
# - each element is itself a list of `variant_settings`, for a particular variant
variant_arrays = variations.map{ |var| var[:variants] }
variant_settings_list = variant_arrays.first.product *variant_arrays[1..]
# binding.pry


### PRODUCE COMPACT FILES **************************************
print_status 'loop over variants'
cleanup_list = []
simulations  = []
variant_settings_list.each_with_index do |variant_settings,variant_id|

  # clone the xml tree
  xml_clone = xml.dup

  # in `xml_clone`, set each attribute of this variant's settings, along with the fixed settings
  settings = variant_settings + fixed_settings
  print_status "-----> setting variant #{variant_id}:"
  ap settings
  settings.each do |var|
    node = xml_clone.at_xpath var[:xpath]
    node.set_attribute var[:attribute], var[:value]
  end

  # create drich compact file `compact_drich`, by writing `xml_clone`
  # - this is a modification of `CompactFile`, with this variant's attributes set
  # - `compact_drich` is written to `#{Detector}/compact`, and copied to `OutputDir`
  basename = File.basename(CompactFile,'.xml') + "_variant#{variant_id}"
  compact_drich = "#{File.dirname(CompactFile)}/#{basename}.xml"
  print_status "produce compact file variant #{compact_drich}"
  File.open(compact_drich,'w') { |out| out.puts xml_clone.to_xml }
  FileUtils.cp compact_drich, "#{OutputDir}/compact"
  cleanup_list << compact_drich

  # create detector template config
  # - this will be combined with `compact_drich` to render the full detector compact file
  config_drich = "#{OutputDir}/config/#{basename}.yml"
  print_status "produce jinja2 config #{config_drich}"
  File.open(config_drich,'w') do |out|
    out.puts <<~EOF
      features:
        pid:
          drich: #{compact_drich.gsub(/^#{Detector}\//,'')}
    EOF
  end

  # render the full detector compact file, `compact_detector`
  # - it will include `compact_drich` instead of the default `CompactFile`
  compact_detector = "#{Detector}/#{Detector}_#{basename}.xml"
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

  # build simulation command
  simulation_output = "#{OutputDir}/sim/#{basename}.root"
  simulations << {
    :id        => variant_id,
    :log       => "#{OutputDir}/log/#{basename}",
    :pipelines => simulation_pipelines.call(compact_detector,simulation_output),
  }

end


### EXECUTION **************************************************

# run the commands listed in `sim[:pipelines]`, and log to `sim[:log]`
def execute_thread(sim)
  print_thread_status = Proc.new do |message|
    puts "-> variant #{sim[:id]} -> #{message}"
  end
  print_thread_status.call "BEGIN"
  sim[:pipelines].each do |simulation_pipeline|
    print_thread_status.call simulation_pipeline.map(&:first).join(' | ')
    Open3.pipeline(
      *simulation_pipeline,
      :out=>["#{sim[:log]}.out",'a'],
      :err=>["#{sim[:log]}.err",'a'],
    )
  end
  print_thread_status.call "END"
end

# execute the threads, either single- or multi-threaded
print_status 'SIMULATION COMMAND (for one variant):'
ap simulations.first
print_status 'begin simulation '.upcase + '='*40
if MultiThreaded
  print_status "running multi-threaded with PoolSize = #{PoolSize}"
  pool = Thread.pool(PoolSize)
  simulations.each do |simulation|
    pool.process{ execute_thread simulation }
  end
  pool.shutdown
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
    FileUtils.rm file, verbose: true
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
