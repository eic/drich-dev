#!/usr/bin/env ruby
#--------------------------------------------------------------------#
# vary parameters of dRICH geometry, and run simulations in parallel #
# Author: C. Dilks                                                   #
#--------------------------------------------------------------------#

require 'awesome_print'
require 'nokogiri'
require 'fileutils'
require 'open3'
require 'pry'


### SETTINGS *************************************
Detector      = 'ecce'                          # path to detector repository
CompactFile   = "#{Detector}/compact/drich.xml" # compact file to vary
Cleanup       = false                           # if true, remove transient files from `#{Detector}/`
MultiThreaded = true                            # if true, run one simulation job per thread
PoolSize      = [`nproc`.to_i-2,1].max          # number of parallel threads to run (`if MultiThreaded`)


### ARGUMENTS ****************************************
OutputDirMain = 'out'
VariatorDir   = 'ruby/variator'
variator_code = 'var0'
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
variant_arrays = variator.varied_settings.map{ |var| var[:variants] }
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
  settings = variant_settings + variator.fixed_settings
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
    :pipelines => variator.simulation_pipelines.call(compact_detector,simulation_output),
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
# - todo: use concurrency instead of fixed pool sizes (current implementation
#         is only efficient if all threads take the same time to run)
print_status 'SIMULATION COMMAND (for one variant):'
ap simulations.first
print_status 'begin simulation '.upcase + '='*40
if MultiThreaded
  print_status "running multi-threaded with PoolSize = #{PoolSize}"
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
