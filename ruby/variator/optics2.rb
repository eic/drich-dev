# optics tuning
require './ruby/variator/variator_base.rb'

class Variator < VariatorBase
  def initialize
    super

    @varied_settings = [
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_x',
        :function  => @@center_delta,
        :args      => [-10, 10],
        :count     => 5,
      },
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_z',
        :function  => @@center_delta,
        :args      => [15, 15],
        :count     => 5,
      },
      {
        :xpath     => '//sensors//sphere',
        :attribute => 'radius',
        :function  => @@center_delta,
        :args      => [100, 20],
        :count     => 5,
        :label     => :sensor_sphere_radius,
      },
    ]

    @derived_settings = [
      {
        :xpath      => '//sensors//sphere',
        :attribute  => 'centerz',
        :derivation => Proc.new{ |value| 50.0 - value[:sensor_sphere_radius] },
      },
    ]

    @simulation_pipelines = @@test_optics

    # best variants:
    #
    # choice 1: {24,32,37,38,43,44,49,64,69,76,77,83,84,88,89,99,102,103,109}
    # choice 2: {32,37,76,77,102}
    #
    # 77 is the best:
    # focus_tune_x          = -5
    # focus_tune_z          = 0
    # sensor_sphere_radius  = 100
    # sensor_sphere_centerz = -50
    # 
    # corresponding mirror parameters (dumped from `scripts/create_irt_auxfile.sh`):
    # SECTOR 0 MIRROR:
    #   mirror x = 113.888311 cm
    #   mirror y = 0.000000 cm
    #   mirror z = 94.364601 cm
    #   mirror R = 218.535399 cm

  end
end
