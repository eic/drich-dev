#!/usr/bin/env ruby

# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2023 Christopher Dilks

require 'yaml'
require 'optparse'
require 'ostruct'

# default CLI options
options = OpenStruct.new
options.sim_file     = 'out/sim.edm4hep.root'
options.rec_file     = 'out/rec.edm4hep.root'
options.ana_file     = ''
options.config_file  = 'config/recon_irt.yaml'
options.dry_run      = false
options.debug_run    = false
options.eicrecon_bin = 'eicrecon'

# convert rec_file to ana_file name
def rec2ana(name)
  name.sub /\.edm4hep\.root$|\.root$/, '.ana\0'
end

# parse CLI options
OptionParser.new do |o|
  o.banner = "USAGE: #{$0} [OPTIONS]..."
  o.separator('')
  o.separator('OPTIONS:')
  o.on("-c", "--config [FILE]", "Configuration YAML file", "Default: #{options.config_file}"){ |a| options.config_file = a }
  o.separator('')
  o.on("-s", "--sim [FILE]", "Simulation input file", "Default: #{options.sim_file}"){ |a| options.sim_file = a }
  o.separator('')
  o.on("-r", "--rec [FILE]", "Reconstruction output file", "Default: #{options.rec_file}"){ |a| options.rec_file = a }
  o.separator('')
  o.on("-a", "--ana [FILE]", "Analysis output file", "Default: #{rec2ana options.rec_file}"){ |a| options.ana_file = a }
  o.separator('')
  o.on("-t", "--dry-run", "Dry run: just print the EICrecon command and exit"){ options.dry_run = true }
  o.separator('')
  o.on("-D", "--debug", "Run in GDB debugger"){
    options.debug_run    = true
    options.eicrecon_bin = 'gdb --args eicrecon'
  }
  o.separator('')
  o.on_tail("-h", "--help", "Show this message") do
    puts o
    exit 2
  end
end.parse!(ARGV)
# puts "OPTIONS: #{options}"

# set hist file, if not set from CLI
ana_file = rec2ana options.rec_file if options.ana_file.empty?

# check for existence of input files
[
  options.sim_file,
  options.config_file,
].each do |name|
  unless File.exist? name
    $stderr.puts "ERROR: file '#{name}' does not exist"
    exit 1
  end
end

# parse configuration file to Hash
config_yaml = YAML.load_file options.config_file

# parse a YAML tree of settings, returning an Array of strings with:
# - list of node path keys combined with `String.join ':'`
# - leaf node appended as "=#{leaf}"
#   - Array leaves will be returned as `String.join ','`
def traverse(tree, tree_name='')
  case tree.class.name
  when 'Hash' # if a sub-tree, recurse
    tree.map do |branch_name, branch|
      next_name = [tree_name, branch_name].join(':').sub /^:/, ''
      traverse branch, next_name
    end.flatten
  when 'Array' # if an array, return Array.join ','
    "#{tree_name}=#{tree.join ','}"
  else # if a leaf, append it to the final string and stop recursion
    [tree_name, tree.to_s].join '='
  end
end

# convert parsed config file settings into 'key=value' pairs JANA can use
arg_list = traverse config_yaml

# fix: key name of log level settings
arg_list.map! do |it|
  if it.match? /^log_levels:/
    it.sub(/^log_levels:/,'').sub(/\=/,':LogLevel=')
  else
    it
  end
end

# append CLI settings
arg_list += traverse({
  "podio:output_file" => options.rec_file,
  "histsfile"         => ana_file,
})

# if debugging, override the timeout
arg_list += traverse({ "jana:timeout" => "0" }) if options.debug_run

# prepend '-P' to each setting, and add quotes around the value
arg_list.map! do |it|
  '-P' + it.sub(/.*\=/, '\0"') + '"'
end

# finally, append the input file name
arg_list << options.sim_file

# build the eicrecon command
eicrecon_cmd = [options.eicrecon_bin, *arg_list].join ' '

# print eicrecon command
puts """
EICRECON ARGUMENTS: ["""
arg_list.each{ |it| puts "  #{it}," }
puts """]

EICRECON COMMAND:
#{eicrecon_cmd}
"""

# if a dry run, exit prematurely
if options.dry_run
  puts "\nThis is a dry run: stopping before running the above command\n\n"
  exit
end

# run eicrecon: `exec` hands process control over to `eicrecon_cmd`
exec eicrecon_cmd
