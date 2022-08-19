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
        :args      => [10, 20],
        :count     => 5,
      },
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_z',
        :function  => @@center_delta,
        :args      => [30, 30],
        :count     => 5,
      },
      {
        :xpath     => '//sensors//sphere',
        :attribute => 'radius',
        :function  => @@min_max,
        :args      => [80, 160],
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
    # 0
    # 6 <- best
    # 7
    # 12
    # 18
    # 32
    # 52
    # 57
    #
    # 6 is the best:
    # focus_tune_x          = -10.0
    # focus_tune_z          = 15.0
    # sensor_sphere_radius  = 100.0
    # sensor_sphere_centerz = -50.0

  end
end
