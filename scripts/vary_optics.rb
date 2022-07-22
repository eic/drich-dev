#!/usr/bin/env ruby
# vary parameters for dRICH optics

require 'numpy'
require 'awesome_print'
require 'nokogiri'
require 'fileutils'
require 'thread/pool'
require 'pry'


### SETTINGS *************************************
Detector      = 'ecce'                          # path to detector repository
CompactFile   = "#{Detector}/compact/drich.xml" # compact file to vary
MultiThreaded = false                           # if true, run one simulation job per thread
Cleanup       = false                           # if true, remove transient files from `#{Detector}/`


### SIMULATION COMMAND ***************************
# - Array representing the simulation command and arguments
# - a full-detector compact file will be created for each variant,
#   thus the simulation command should involve `compact_file`
# - a unique `output_file` name must also be specified
simulation_command = Proc.new do |compact_file,output_file|
  [
    # "exit|",
    # "echo",
    "./simulate.py",
    "-t 1",
    # "-t 12",
    # "-v",
    # "-m svg",
    "-c #{compact_file}",
    "-o #{output_file}",
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
    :count     => 1,
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
  { :constant=>'DRICH_debug_optics', :value=>'1' },
]


#####################################################################
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
def status(message)
  puts "[***] #{message}"
end
status 'preparation'

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
[ 'compact', 'config', 'sim' ].each do |subdir|
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
status 'loop over variants'
cleanup_list    = []
simulation_list = []
variant_settings_list.each_with_index do |variant_settings,variant_id|

  # clone the xml tree
  xml_clone = xml.dup

  # in `xml_clone`, set each attribute of this variant's settings, along with the fixed settings
  settings = variant_settings + fixed_settings
  status "-----> setting variant #{variant_id}:"
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
  status "produce compact file variant #{compact_drich}"
  File.open(compact_drich,'w') { |out| out.puts xml_clone.to_xml }
  FileUtils.cp compact_drich, "#{OutputDir}/compact"
  cleanup_list << compact_drich

  # create detector template config
  # - this will be combined with `compact_drich` to render the full detector compact file
  config_drich = "#{OutputDir}/config/#{basename}.yml"
  status "produce jinja2 config #{config_drich}"
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
  status "jinja2 render template to #{compact_detector}"
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
  simulation_list << simulation_command.call(compact_detector,simulation_output)

end


### EXECUTION **************************************************

# run jobs
status 'simulation commands:'.upcase
ap simulation_list
status 'begin simulation '.upcase + '='*40
if MultiThreaded
  pool_size = [`nproc`.to_i-2,1].max # nCPUs-2
  puts "thread pool size = #{pool_size}"
  pool = Thread.pool(pool_size)
  simulation_list.each do |simulation|
    pool.process{ system simulation.join(' ') }
  end
  pool.shutdown
else
  simulation_list.each do |simulation|
    system simulation.join(' ')
  end
end

# cleanup the transient compact files
if Cleanup
  status "cleanup transient files:"
  ap cleanup_list.sort
  cleanup_list.each do |file|
    FileUtils.rm file, verbose: true
  end
end

status 'DONE'
if errors.size>0
  status 'ERRORS:'
  ap errors
else
  status 'NO ERRORS'
end
