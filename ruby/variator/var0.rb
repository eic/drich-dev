require './ruby/variator/functions.rb'

class Variator < VariationFunctions
  def initialize

    ### PARAMETER VARIATIONS *************************
    @varied_settings = [
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_x',
        :function  => @@center_delta,
        :args      => [70, 20],
        :count     => 4,
      },
      # {
      #   :xpath     => '//mirror',
      #   :attribute => 'focus_tune_z',
      #   :function  => @@min_max,
      #   :args      => [20, 50],
      #   :count     => 3,
      # },
    ]

    ### FIXED SETTINGS *******************************
    @fixed_settings = [
      # { :constant=>'DRICH_debug_optics', :value=>'1' },
    ]

    ### SIMULATION COMMANDS **************************
    @simulation_pipelines = Proc.new do |compact_file,output_file|
      [
        [[
          "./simulate.py",
          "-t 1",
          "-n 100",
          "-c #{compact_file}",
          "-o #{output_file}",
        ]],
        [[
          "./drawHits.exe",
          output_file,
        ]],
        # [
        #   ["exit"],
        #   [
        #     "echo",
        #     "./simulate.py",
        #     "-t12",
        #     "-v",
        #     "-msvg",
        #     "-c#{compact_file}",
        #     "-o#{output_file}",
        #   ],
        # ],
      ]
    end

    ### **********************************************

  end
  attr_accessor :varied_settings, :fixed_settings, :simulation_pipelines
end
