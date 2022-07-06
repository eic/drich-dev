#!/usr/bin/env ruby

# generate list of parameters
listA = [1,2,3]
listB = [10,20,30]

# loop over all combinations of parameters
listA.product.listB.each do |a,b|

  testName="drich_test_#{a}_#{b}"

  # cp compact/drich.xml to compact/{testName}.xml

  # parse XML file compact/{testName}.xml, and set A and B as desired

  # create yml file, named /path/to/somewhere/{testName}.yml, that looks like:
  """
  features:
  pid:
    drich: compact/#{testName}.xml
  """

  # parse with
  """
  bin/make_detector_configuration \
    -d templates \
    -t ecce.xml.jinja2 \
    -o ecce_#{testName}.xml \
    -c /path/to/somewhere/#{testName}.yml
  """

  # run npsim with the resulting `ecce_{testName}.xml` file; ideally
  # one thread per npsim job, each with a different A and B

end

# cleanup the transient yml and xml files
