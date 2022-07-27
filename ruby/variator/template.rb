# Template variator code
# - use this to start your own variation study
# - see other `var*.rb` files for more examples

require './ruby/variator/variator_base.rb'

class Variator < VariatorBase
  def initialize

    ### PARAMETER VARIATIONS *************************
    # - create the following Hash for each variation, and add it to the 
    #   Array `@varied_settings`:
    #     {
    #       :xpath     => XPATH to the XML node
    #       :attribute => node attribute
    #       :function  => variation function (see variator_base.rb)
    #       :args      => variation arguments
    #       :count     => number of variations
    #     }
    #
    # - note: to vary a "constant" such as `DRICH_Length` in the compact file, use:
    #     { :xpath=>'//constant[@name="DRICH_Length"]', :attribute=>'value' }
    #   
    @varied_settings = [
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_x',
        :function  => @@center_delta,
        :args      => [70, 20],
        :count     => 3,
      },
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_z',
        :function  => @@min_max,
        :args      => [30, 40],
        :count     => 2,
      },
    ]


    ### FIXED SETTINGS *******************************
    # specify specific fixed settings, with similar Hashes, either of:
    #   { :constant, :value }           # for `XPATH=//constant` nodes
    #   { :xpath, :attribute, :value }  # for general attribute
    @fixed_settings = [
      { :constant=>'DRICH_debug_optics', :value=>'0' },
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
          "./drawHits.exe",
          settings[:output],
        ]],
      ]
    end


    ### **********************************************

  end
  attr_accessor :varied_settings, :fixed_settings, :simulation_pipelines
end
