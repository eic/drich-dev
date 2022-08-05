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
        :args      => [0, 20],
        :count     => 3,
      },
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_z',
        :function  => @@center_delta,
        :args      => [30, 30],
        :count     => 3,
      },
      {
        :xpath     => '//sensors//sphere',
        :attribute => 'radius',
        :function  => @@center_delta,
        :args      => [140, 40],
        :count     => 3,
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
    # 3:  sensorsphere radius 100, focus tune (x,z)=(-20,30)
    # 9:  sensorsphere radius 100, focus tune (x,z)=(0,0)
    #
    # 7:  sensorsphere radius 140, focus tune (x,z)=(-20,60)
    # 13: sensorsphere radius 140, focus tune (x,z)=(0,30)

  end
end
