require './ruby/variator/variator_base.rb'

class Variator < VariatorBase
  attr_accessor :varied_settings, :fixed_settings, :simulation_pipelines

  def initialize
    super

    @varied_settings = [
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_x',
        :function  => @@center_delta,
        :args      => [0, 20],
        :count     => 4,
      },
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_z',
        :function  => @@center_delta,
        :args      => [30, 30],
        :count     => 3,
      },
    ]

    @simulation_pipelines = @@test_optics

  end
end
