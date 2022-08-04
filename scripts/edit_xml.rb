#!/usr/bin/env ruby
# quickly change a parameter in an XML file

require 'nokogiri'
require 'fileutils'

if ARGV.length != 4
  $stderr.puts """
  USAGE: #{$0} [XML_FILE] [XPATH] [ATTRIBUTE] [NEW VALUE]

    NOTE: this will backup the original XML_FILE, then it
          will OVERWRITE the original XML_FILE
    - this script is meant to be used for batch processing
    - the xml file will be re-formatted automatically
    - if you lose an important xml file, use git to save you
  """
  exit 2
end
xml_file, xpath, attribute, value = ARGV

# backup original xml_file
xml_file_bak = xml_file + '.bak'
FileUtils.mv xml_file, xml_file_bak, verbose: true

# parse
xml = Nokogiri::XML File.open(xml_file_bak)

# get nodes for given xpath
nodes = xml.xpath xpath
if nodes.size == 0
  $stderr.puts "ERROR: cannot find node at xpath '#{xpath}'"
  exit 1
elsif nodes.size > 1
  $stderr.puts "WARNING: more than one node for xpath '#{xpath}'"
end

# set attribute's value for each node
nodes.each do |node|
  node.set_attribute attribute, value
end

# write
File.open(xml_file,'w') { |out| out.puts xml.to_xml }
puts "wrote new #{xml_file}"
