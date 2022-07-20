#!/usr/bin/env ruby
# vary parameters for dRICH optics

require 'numpy'
require 'awesome_print'


### VARIATION FUNCTIONS **************************
# linearly vary by `init +/- delta`, `count` times
center_delta = Proc.new do |center, delta, count|
  Numpy.linspace(center-delta, center+delta, count)
end
# linearly vary from `min` to `max`, `count` times
min_max = Proc.new do |min, max, count|
  Numpy.linspace(min, max, count)
end


### PARAMETER VARIATIONS *************************
variations = [
  {
    :name      => 'mirror/focus_tune_x',
    :units     => 'cm',
    :variation => center_delta,
    :args      => [70, 20],
    :count     => 3,
  },
  {
    :name      => 'mirror/focus_tune_y',
    :units     => 'cm',
    :variation => min_max,
    :args      => [20, 50],
    :count     => 3,
  },
]


### **********************************************

# for each variation in `variation`, add key `:arr`, pointing to an Array
# of variation values; we will call this `arr` below
variations.each do |var|
  var[:arr] = var[:variation]
    .call(*var[:args], var[:count], var[:units])
    .to_a
    .map{ |val| "#{val}*#{var[:units]}" }
end
ap variations

# build a list of lists of variations, used for the next step
# - uncomment `ap` to print (easier seen than described)
# - Hashes are used to store the variable name with the value
variation_arrs = variations.map do |var|
  var[:arr].map do |val|
    { 
      :name => var[:name],
      :val  => val,
    }
  end
end
# ap variation_arrs

# build a list of all the possible variations by taking the product 
# of all the arrs
variation_product = variation_arrs.first.product(*variation_arrs[1..])
# ap variation_product




# # loop over all combinations of parameters
# listA.product.listB.each do |a,b|

#   testName="drich_test_#{a}_#{b}"

#   # cp compact/drich.xml to compact/{testName}.xml

#   # parse XML file compact/{testName}.xml, and set A and B as desired

#   # create yml file, named /path/to/somewhere/{testName}.yml, that looks like:
#   """
#   features:
#   pid:
#     drich: compact/#{testName}.xml
#   """

#   # parse with
#   """
#   bin/make_detector_configuration \
#     -d templates \
#     -t ecce.xml.jinja2 \
#     -o ecce_#{testName}.xml \
#     -c /path/to/somewhere/#{testName}.yml
#   """

#   # run npsim with the resulting `ecce_{testName}.xml` file; ideally
#   # one thread per npsim job, each with a different A and B

# end

# # cleanup the transient yml and xml files
