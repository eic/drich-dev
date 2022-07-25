require 'numpy'

class VariationFunctions

  ### VARIATION FUNCTIONS **************************
  # - add any variation functions here
  # - must return Array
  # - python methods are available via the 'pycall' gem (such as 'numpy')

  # linearly vary by `center +/- delta`, `count` times
  @@center_delta = Proc.new do |center, delta, count|
    Numpy.linspace(center-delta, center+delta, count).to_a
  end

  # linearly vary from `min` to `max`, `count` times
  @@min_max = Proc.new do |min, max, count|
    Numpy.linspace(min, max, count).to_a
  end

end
