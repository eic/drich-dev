# Template variator code
# - use this to start your own variation study
# - see other `var*.rb` files for more examples
# - here we define the class `Variator`, which inherits from `VariatorBase`,
#   where we only need to write the constructor `initialize`:
#   - first call `VariatorBase` constructor (`super`) to instantiate instance
#     variables such as `@varied_settings` (they all start with a single `@`)
#   - overwrite the instance variables with your own

##### begin boilerplate #####
require './ruby/variator/variator_base.rb'
class Variator < VariatorBase
  def initialize
    super
    ##### end boilerplate #####


    ### PARAMETER VARIATIONS *************************
    # - create the following Hash for each variation, and add it to the 
    #   Array `@varied_settings`:
    #     {
    #       :xpath     => XPATH to the XML node
    #       :attribute => node attribute
    #       :function  => variation function (see variator_base.rb)
    #       :args      => variation arguments
    #       :count     => number of variations
    #       :label     => OPTIONAL: unique Symbol representing this variation; see `@derived_settings` below
    #     }
    # - XPATHs can be specific or more general; see https://nokogiri.org/tutorials/searching_a_xml_html_document.html
    # - try to make sure the XPATH refers to a unique node
    # - note: to vary a "constant" such as `DRICH_Length` in the compact file, use:
    #     { :xpath=>'//constant[@name="DRICH_Length"]', :attribute=>'value' }
    #   
    @varied_settings = [
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_z',
        :function  => @@min_max,
        :args      => [30, 40],
        :count     => 2,
      },
      {
        :xpath     => '//sensors//sphere',
        :attribute => 'radius',
        :function  => @@center_delta,
        :args      => [140, 20],
        :count     => 3,
        :label     => :sensor_sphere_radius,
      },
    ]


    ### FIXED SETTINGS *******************************
    # - specify specific fixed settings
    # - `@fixed_settings` is also an array of Hashes, with one of the following forms:
    #     { :constant, :value }           # for `XPATH=//constant` nodes
    #     { :xpath, :attribute, :value }  # for general attribute (similar to `@varied_settings`)
    # - this is optional, don't set it if you don't need it
    @fixed_settings = [
      { :constant=>'DRICH_debug_optics', :value=>'0' },
    ]


    ### DERIVED SETTINGS *****************************
    # specify settings which depend on variant-specific values
    # - `@derived_settings` is also an array of Hashes
    # - add a unique `:label` to any variation setting in `@varied_settings`;
    #   the label will provide access to that variation's variant-specific value
    # - the Hash is defined as:
    #     {
    #       :xpath      => XPATH to the XML node to set
    #       :attribute  => node attribute to set
    #       :derivation => a Proc, which returns the derived value you want to set (see below)
    #     }
    # - the `:derivation` Proc takes one argument, a Hash which contains
    #   key=>value pairs where the keys are the `:label`s you defined in
    #   `@varied_settings`, and the values are the variant-specific values
    # - this is optional, don't set it if you don't need it
    @derived_settings = [
      # sets sensor sphere `centerz` to `50 - radius`
      {
        :xpath      => '//sensors//sphere',
        :attribute  => 'centerz',
        :derivation => Proc.new{ |value| 50.0 - value[:sensor_sphere_radius] },
      },
    ]


    ### SIMULATION COMMANDS **************************
    # - list of commands to run the simulation
    # - the full `simulation_pipelines` array is a list of pipelines, which will be
    #   executed sequentially
    #   - a pipeline is a list of commands, where stdout of one command is streamed
    #     to stdin of the next command
    #     - each command is written as an array, where the first element is the
    #       command, and the remaining elements are its arguments
    #     - a pipeline can be a single command
    #   - the list of pipelines will be executed for each variant
    #   - example pipelines:
    #     [[ "ls", "-t" ]]                  # => `ls -t`
    #     [ ["ls","-lt"], ["tail","-n3"] ]  # => `ls -lt | tail -n3`
    #
    # - `settings` is a Hash, including the following:
    #     {
    #       :id               => variant ID
    #       :compact_drich    => dRICH compact file; this file is the variant dRICH configuration
    #       :compact_detector => full detector compact file; references settings[:compact_drich]
    #       :output           => output ROOT file
    #       :log              => output log file prefix
    #       :variant_info     => Hash of this variant's settings
    #     }
    #
    # - some common `simulation_pipelines` are found in `variator_base.rb`,
    #   for example, optics testing; you are welcome to add your own there too
    #
    @simulation_pipelines = Proc.new do |settings|
      [
        [[
          "./simulate.py",
          "-t 1",
          "-n 100",
          "-c #{settings[:compact_detector]}",
          "-o #{settings[:output]}",
        ]],
        [[
          "bin/draw_hits",
          settings[:output],
        ]],
      ]
    end


  ##### begin boilerplate #####
  end # `Variator` constructor
end # class `Variator`
