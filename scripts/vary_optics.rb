#!/usr/bin/env ruby
# vary parameters for dRICH optics

require 'numpy'
require 'awesome_print'
require 'nokogiri'
require 'fileutils'
require 'thread/pool'
require 'pry'

CompactFile = '/home/dilks/tmp/drich.xml'  # todo
OutputDir   = 'out/optics'
Detector    = 'ecce'

### VARIATION FUNCTIONS **************************
# - must return numpy array

# linearly vary by `center +/- delta`, `count` times
center_delta = Proc.new do |center, delta, count|
  Numpy.linspace(center-delta, center+delta, count)
end

# linearly vary from `min` to `max`, `count` times
min_max = Proc.new do |min, max, count|
  Numpy.linspace(min, max, count)
end


### PARAMETER VARIATIONS *************************
# create the following Hash for each variation, and
# add it to the Array `variations`:
# {
#   :xpath     => XPATH to the XML node
#   :attribute => node attribute
#   :variation => variation function (see above)
#   :args      => variation arguments
#   :count     => number of variations
# }
variations = [
  {
    :xpath     => '//mirror',
    :attribute => 'focus_tune_x',
    :variation => center_delta,
    :args      => [70, 20],
    :count     => 3,
  },
  {
    :xpath     => '//mirror',
    :attribute => 'focus_tune_y',
    :variation => min_max,
    :args      => [20, 50],
    :count     => 3,
  },
]


### FIXED SETTINGS *******************************
# specify specific fixed settings, with similar Hashes, either of:
# { :constant, :value }           # for `XPATH=//constant` nodes
# { :xpath, :attribute, :value }  # for general attribute
fixed_settings = [
  { :constant=>'DRICH_debug_optics', :value=>'1' },
]


#####################################################################
#####################################################################


### PREPARATION **************************************

# make output directories
[ 'compact', 'config' ].each do |subdir|
  FileUtils.mkdir_p "#{OutputDir}/#{subdir}"
end

# set xpaths for constant settings
fixed_settings.each do |setting|
  if setting.has_key? :constant
    setting[:xpath] = "//constant[@name=\"#{setting[:constant]}\"]"
    setting[:attribute] = 'value'
  end
end
binding.pry

# parse compact file
xml = Nokogiri::XML File.open(CompactFile)

# for each variation in `variation`, add key `:arr`, pointing to an Array
# of variation values; we will call this Array "arr" below
variations.each do |var|
  # get units
  nodes = xml.xpath var[:xpath]
  $stderr.puts "WARNING: more than one node for xpath #{var[:xpath]}" if nodes.size>0  # todo: not yet supported
  val_str = nodes.first.attr var[:attribute]
  units = val_str.include?('*') ?
    '*' + val_str.split('*').last :
    ''
  # call var[:variation] function
  var[:arr] = var[:variation].call(*var[:args], var[:count])
    .to_a
    .map{ |val| "#{val}#{units}" }
end
binding.pry

# map list of variations to list of arrs, and
# map each arr to a list of hashes, including the variation value and xpath
variation_arrs = variations.map do |var|
  var[:arr].map do |val|
    { 
      :xpath     => var[:xpath],
      :attribute => var[:attribute],
      :value     => val,
    }
  end
end
binding.pry

# build a list of all the possible variations (product of all arrs)
arrs_product = variation_arrs.first.product *variation_arrs[1..]
binding.pry


### PRODUCE COMPACT FILES **************************************
cleanup_list = []
job_list     = []
arrs_product.each_with_index do |vars,job_id|

  # clone the xml tree
  xml_clone = xml.dup

  # set each attribute of the variation, along with fixed settings
  settings = vars + fixed_settings
  settings.each do |var|
    node = xml_clone.at_xpath var[:xpath]
    node.set_attribute var[:attribute], var[:value]
  end

  # create drich compact file `compact_drich`
  # - this is a modification of `CompactFile`, with this variation's
  #   attributes set
  # - `compact_drich` is written to `#{Detector}/compact`, and
  #   copied to `OutputDir`
  basename = File.basename(CompactFile,'.xml') + "__#{job_id}"
  compact_drich = "#{File.dirname(CompactFile)}/#{basename}.xml"
  File.open(compact_drich) do |out|
    out.puts xml_clone.to_xml
  end
  FileUtils.cp compact_drich, "#{OutputDir}/compact"
  cleanup_list << compact_drich

  # create detector template config
  # - this will be combined with `compact_drich` to produce the full
  #   detector compact file
  config = "#{OutputDir}/config/#{basename}.yml"
  File.open(config) do |out|
    out.puts <<~EOF
      features:
        pid:
          drich: #{compact_drich}
    EOF
  end

  # produce full detector compact file, `compact_detector`
  # - `compact_detector` will include `compact_drich` instead of `CompactFile`
  compact_detector = "#{Detector}/#{Detector}__#{job_id}.xml"
  system *[
    "#{Detector}/bin/make_detector_configuration",
    "-d #{Detector}/templates",
    "-t #{Detector}.xml.jinja2",
    "-o #{compact_detector}",
    "-c #{config}",
  ]
  cleanup_list << compact_detector

  # build simulation command
  job_list << [
    "./simulate.py -t10",
    "-c #{compact_detector}", # todo
  ].join(' ')

end

binding.pry
exit

# run jobs
pool_size = [`nproc`.to_i-2,1].max # nCPUs-2
puts "thread pool size = #{poolSize}"
pool = Thread.pool(pool_size)
job_list.each do |job|
  pool.process{ system job }
end
pool.shutdown

# cleanup the transient compact files
cleanup_list.each do |file|
  FileUtils.rm file, verbose: true
end
