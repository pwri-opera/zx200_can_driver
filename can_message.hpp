#pragma onece

#include <dbcppp/Network.h>
#include <dbcppp/Network2Functions.h>

#include <rttr/registration>

struct pilot_pressure_cmd1
{
  pilot_pressure_cmd1(){};
  void func(double){};
  double boom_up;
  double boom_down;
  double arm_crowd;
  double arm_dump;
  double bucket_crowd;
  double bucket_dump;
  double swing_right;
  double swing_left;
};
struct pilot_pressure_cmd2
{
  pilot_pressure_cmd2(){};
  void func(double){};
  double track_right_forward;
  double track_left_forward;
  double track_right_backward;
  double track_left_backward;
  double attachment_a;
  double attachment_b;
  double assist_a;
  double assist_b;
};

struct machine_setting_cmd
{
  // machine_setting_cmd(){};
  void func(double){};
  std::uint16_t engine_rpm;
  bool power_eco_mode;
  std::uint8_t travel_speed_mode;
  bool working_mode_notice;
  std::uint8_t yellow_led_mode;
  // byte3-7 is not assigned
  std::uint8_t alive_counter;
};

struct Front_Angle
{
  double boom_relative_angle;
  double arm_relative_angle;
  double bucket_relative_angle;
  double swing_relative_angle;
  RTTR_ENABLE()
};

struct Can_Bus : Front_Angle
{
  RTTR_ENABLE(Front_Angle)
};

RTTR_REGISTRATION
{
  rttr::registration::class_<pilot_pressure_cmd1>("pilot_pressure_cmd1")
      .constructor<>()
      .property("boom_up", &pilot_pressure_cmd1::boom_up)
      .property("boom_down", &pilot_pressure_cmd1::boom_down)
      .property("arm_crowd", &pilot_pressure_cmd1::arm_crowd)
      .property("arm_dump", &pilot_pressure_cmd1::arm_dump)
      .property("bucket_crowd", &pilot_pressure_cmd1::bucket_crowd)
      .property("bucket_dump", &pilot_pressure_cmd1::bucket_dump)
      .property("swing_right", &pilot_pressure_cmd1::swing_right)
      .property("swing_left", &pilot_pressure_cmd1::swing_left)
      .method("func", &pilot_pressure_cmd1::func);

  rttr::registration::class_<pilot_pressure_cmd2>("pilot_pressure_cmd2")
      .constructor<>()
      .property("track_right_forward", &pilot_pressure_cmd2::track_right_forward)
      .property("track_left_forward", &pilot_pressure_cmd2::track_left_forward)
      .property("track_right_backward", &pilot_pressure_cmd2::track_right_backward)
      .property("track_left_backward", &pilot_pressure_cmd2::track_left_backward)
      .property("attachment_a", &pilot_pressure_cmd2::attachment_a)
      .property("attachment_b", &pilot_pressure_cmd2::attachment_b)
      .property("assist_a", &pilot_pressure_cmd2::assist_a)
      .property("assist_b", &pilot_pressure_cmd2::assist_b)
      .method("func", &pilot_pressure_cmd2::func);

  rttr::registration::class_<machine_setting_cmd>("machine_setting_cmd")
      .constructor<>()
      .property("engine_rpm", &machine_setting_cmd::engine_rpm)
      .property("power_eco_mode", &machine_setting_cmd::power_eco_mode)
      .property("travel_speed_mode", &machine_setting_cmd::travel_speed_mode)
      .property("working_mode_notice", &machine_setting_cmd::working_mode_notice)
      .property("yellow_led_mode", &machine_setting_cmd::yellow_led_mode)
      .property("alive_counter", &machine_setting_cmd::alive_counter)
      .method("func", &machine_setting_cmd::func);

  rttr::registration::class_<Front_Angle>("Front_Angle")
      .constructor<>()
      .property("Front_Angle::boom_relative_angle", &Front_Angle::boom_relative_angle)
      .property("Front_Angle::arm_relative_angle", &Front_Angle::arm_relative_angle)
      .property("Front_Angle::bucket_relative_angle", &Front_Angle::bucket_relative_angle)
      .property("Front_Angle::swing_relative_angle", &Front_Angle::swing_relative_angle);

  rttr::registration::class_<Can_Bus>("can_bus")
      .constructor<>();
}

