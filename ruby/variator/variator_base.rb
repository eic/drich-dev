require 'numpy'

# Variator base class, containing common code for all variators
class VariatorBase

  ### VARIATION FUNCTIONS **************************
  # - you are welcome to add any variation functions here
  # - python methods are available via the 'pycall' gem (such as 'numpy')
  # - must return Array (use `.to_a` to convert a numpy array to a ruby Array)

  # linearly vary by `center +/- delta`, `count` times
  @@center_delta = Proc.new do |center, delta, count|
    Numpy.linspace(center-delta, center+delta, count).to_a
  end

  # linearly vary from `min` to `max`, `count` times
  @@min_max = Proc.new do |min, max, count|
    Numpy.linspace(min, max, count).to_a
  end


  ### SIMULATION COMMANDS **************************
  # - common pipeline lists, for example optics testing
  # - you are welcome to add your own here too
  # - see template.rb for syntax details

  ### run some tests for optics optimization
  @@test_optics = Proc.new do |settings|
    [
      ### be sure optics-debugging mode is disabled
      [[ 'scripts/edit_xml.rb', settings[:compact_drich], '//constant[@name="DRICH_debug_optics"]', 'value', '0' ]],
      ### run simulation to check hit rings at varying theta
      [[
        './simulate.py',
        '-t 4',
        '-n 100',
        "-c #{settings[:compact_detector]}",
        "-o #{settings[:output]}",
      ]],
      ### draw the hits
      [[ './drawHits.exe', settings[:output] ]],
      ### enable optics-debugging mode: all components become vacuum, except for mirrors
      [[ 'scripts/edit_xml.rb', settings[:compact_drich], '//constant[@name="DRICH_debug_optics"]', 'value', '1' ]],
      ### visualize parallel-to-point focal region
      #   - NOTE: use Xvfb to suppress OGL windows (`xvfb-run opt/eic-shell`)
      #   - `exit` is piped to automatically exit the interactive G4 shell
      [
        ["exit"],
        [
          "./simulate.py",
          "-t 12",
          "-v",
          "-m svg",
          "-c #{settings[:compact_detector]}",
          "-o #{settings[:output]}",
        ],
      ],
    ]
  end


  ### construtor **************************
  # - instantiates instance variables, which are meant to be overridden in subclasses
  #   (see template.rb)
  def initialize
    @varied_settings  = Array.new
    @fixed_settings   = Array.new
    @derived_settings = Array.new
  end

end
