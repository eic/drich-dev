require './ruby/variator/variator_base.rb'

class Variator < VariatorBase
  def initialize

    @varied_settings = [
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_x',
        :function  => @@center_delta,
        :args      => [70, 20],
        :count     => 4,
      },
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_z',
        :function  => @@min_max,
        :args      => [20, 50],
        :count     => 3,
      },
    ]

    @fixed_settings = []

    @simulation_pipelines = @@test_optics

  end
  attr_accessor :varied_settings, :fixed_settings, :simulation_pipelines
end
