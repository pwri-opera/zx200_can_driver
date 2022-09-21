// This file created by customized dbcppp automatically
// Menbers in structure can be access by reflection using RTTR libraries(https://www.rttr.org/)
// Change Type to correct variable type

#include <dbcppp/Network.h>
#include <dbcppp/Network2Functions.h>
#include <rttr/registration>

struct Machine_State
{
  std::uint8_t alive_counter;
  std::int8_t hydraulic_oil_temp;
  bool engine_state;
  std::uint8_t control_state;
  bool switch_error;
  bool emergency_stop_receiver_error;
  bool lock_receiver_error;
  bool can_error_ict;
  bool can_error_body;
  bool can_error_pl;
  bool zx200x7_system_error;
  bool pilot_shutoff_valve_state;
  std::uint8_t unlock_cmd_state;
  bool lock_cmd_state;
  RTTR_ENABLE()
};
struct Hydraulic_Flow_Rate_2
{
  std::int32_t left_track_motor_b_flow_rate;
  std::int32_t left_track_motor_a_flow_rate;
  std::int32_t right_track_motor_b_flow_rate;
  std::int32_t right_track_motor_a_flow_rate;
  RTTR_ENABLE()
};
struct Hydraulic_Flow_Rate_1
{
  std::int32_t swing_motor_b_flow_rate;
  std::int32_t swing_motor_a_flow_rate;
  std::int32_t bucket_cylinder_rod_flow_rate;
  std::int32_t bucket_cylinder_bottom_flow_rate;
  std::int32_t arm_cylinder_rod_flow_rate;
  std::int32_t arm_cylinder_bottom_flow_rate;
  std::int32_t boom_cylinder_rod_flow_rate;
  std::int32_t boom_cylinder_bottom_flow_rate;
  RTTR_ENABLE()
};
struct Pilot_Pressure_2
{
  double assist_b_pilot_pressure;
  double assist_a_pilot_pressure;
  double attachment_b_pilot_pressure;
  double attachment_a_pilot_pressure;
  double left_track_backward_pilot_prs;
  double left_track_forward_pilot_prs;
  double right_track_backward_pilot_prs;
  double right_track_forward_pilot_prs;
  RTTR_ENABLE()
};
struct Pilot_Pressure_1
{
  double swing_left_pilot_pressure;
  double swing_right_pilot_pressure;
  double bucket_dump_pilot_pressure;
  double bucket_crowd_pilot_pressure;
  double arm_dump_pilot_pressure;
  double arm_crowd_pilot_pressure;
  double boom_down_pilot_pressure;
  double boom_up_pilot_pressure;
  RTTR_ENABLE()
};
struct Pressure_2
{
  double assit_b_pressure;
  double assist_a_pressure;
  double attachment_b_pressure;
  double attachment_a_pressure;
  double left_track_motor_b_pressure;
  double left_track_motor_a_pressure;
  double right_track_motor_b_pressure;
  double right_track_motor_a_pressure;
  RTTR_ENABLE()
};
struct Pressure_1
{
  double swing_motor_b_pressure;
  double swing_motor_a_pressure;
  double bucket_cylinder_rod_pressure;
  double bucket_cylinder_bottom_pressure;
  double arm_cylinder_rod_pressure;
  double arm_cylinder_bottom_pressure;
  double boom_cylinder_rod_pressure;
  double boom_cylinder_bottom_pressure;
  RTTR_ENABLE()
};
struct Vehicle_Azimuth
{
  double track_body_azimuth;
  double upper_body_azimuth;
  RTTR_ENABLE()
};
struct Swing_Center_Position_3
{
  double swing_center_position_z;
  RTTR_ENABLE()
};
struct Swing_Center_Position_2
{
  double swing_center_position_y;
  RTTR_ENABLE()
};
struct Swing_Center_Position_1
{
  double swing_center_position_x;
  RTTR_ENABLE()
};
struct Front_Pin_Position_3
{
  double bucket_pin_position_z;
  double bucket_pin_position_y;
  double bucket_pin_position_x;
  RTTR_ENABLE()
};
struct Front_Pin_Position_2
{
  double arm_pin_position_z;
  double arm_pin_position_y;
  double arm_pin_position_x;
  RTTR_ENABLE()
};
struct Front_Pin_Position_1
{
  double boom_pin_position_z;
  double boom_pin_position_y;
  double boom_pin_position_x;
  RTTR_ENABLE()
};
struct Roll_Pitch_Angle
{
  double pitch_angle;
  double roll_angle;
  RTTR_ENABLE()
};
struct Front_Angular_Velocity
{
  double swing_relative_angular_velocity;
  double bucket_relative_angular_velocity;
  double arm_relative_angular_velocity;
  double boom_relative_angular_velocity;
  RTTR_ENABLE()
};
struct Front_Angle
{
  double bucket_relative_angle;
  double swing_relative_angle;
  double arm_relative_angle;
  double boom_relative_angle;
  RTTR_ENABLE()
};
struct Machine_Setting_Cmd
{
  bool working_mode_notice;
  std::uint8_t yellow_led_mode;
  std::uint8_t travel_speed_mode;
  bool power_eco_mode;
  std::uint8_t alive_counter;
  std::int16_t engine_rpm;
  std::uint64_t Id() { return id; }
  std::uint64_t Cycle_time() { return cycle_time;}
  const std::uint64_t id = 2566874634;
  const std::uint64_t cycle_time = 50;
  RTTR_ENABLE()
};
struct Pilot_Pressure_Cmd_2
{
  double assist_b;
  double assist_a;
  double attachment_b;
  double attachment_a;
  double left_track_backward;
  double right_track_backward;
  double left_track_forward;
  double right_track_forward;
  std::uint64_t Id() { return id; }
  std::uint64_t Cycle_time() { return cycle_time; }
  const std::uint64_t id = 2566874378;
  const std::uint64_t cycle_time = 10;
  RTTR_ENABLE()
};
struct Pilot_Pressure_Cmd_1
{
  double swing_right;
  double swing_left;
  double bucket_dump;
  double bucket_crowd;
  double arm_dump;
  double arm_crowd;
  double boom_down;
  double boom_up;
  std::uint64_t Id() { return id; }
  std::uint64_t Cycle_time() { return cycle_time;}
  const std::uint64_t id = 2566874122;
  const std::uint64_t cycle_time = 10;
  RTTR_ENABLE()
};
struct Can_Bus : Machine_State, Hydraulic_Flow_Rate_2, Hydraulic_Flow_Rate_1, Pilot_Pressure_2, Pilot_Pressure_1, Pressure_2, Pressure_1, Vehicle_Azimuth, Swing_Center_Position_3, Swing_Center_Position_2, Swing_Center_Position_1, Front_Pin_Position_3, Front_Pin_Position_2, Front_Pin_Position_1, Roll_Pitch_Angle, Front_Angular_Velocity, Front_Angle, Machine_Setting_Cmd, Pilot_Pressure_Cmd_2, Pilot_Pressure_Cmd_1
{
  RTTR_ENABLE(Machine_State, Hydraulic_Flow_Rate_2, Hydraulic_Flow_Rate_1, Pilot_Pressure_2, Pilot_Pressure_1, Pressure_2, Pressure_1, Vehicle_Azimuth, Swing_Center_Position_3, Swing_Center_Position_2, Swing_Center_Position_1, Front_Pin_Position_3, Front_Pin_Position_2, Front_Pin_Position_1, Roll_Pitch_Angle, Front_Angular_Velocity, Front_Angle, Machine_Setting_Cmd, Pilot_Pressure_Cmd_2, Pilot_Pressure_Cmd_1)
};

RTTR_REGISTRATION
{
  rttr::registration::class_<Machine_State>("Machine_State")
      .constructor<>()
      .property("Machine_State::alive_counter", &Machine_State::alive_counter)
      .property("Machine_State::hydraulic_oil_temp", &Machine_State::hydraulic_oil_temp)
      .property("Machine_State::engine_state", &Machine_State::engine_state)
      .property("Machine_State::control_state", &Machine_State::control_state)
      .property("Machine_State::switch_error", &Machine_State::switch_error)
      .property("Machine_State::emergency_stop_receiver_error", &Machine_State::emergency_stop_receiver_error)
      .property("Machine_State::lock_receiver_error", &Machine_State::lock_receiver_error)
      .property("Machine_State::can_error_ict", &Machine_State::can_error_ict)
      .property("Machine_State::can_error_body", &Machine_State::can_error_body)
      .property("Machine_State::can_error_pl", &Machine_State::can_error_pl)
      .property("Machine_State::zx200x7_system_error", &Machine_State::zx200x7_system_error)
      .property("Machine_State::pilot_shutoff_valve_state", &Machine_State::pilot_shutoff_valve_state)
      .property("Machine_State::unlock_cmd_state", &Machine_State::unlock_cmd_state)
      .property("Machine_State::lock_cmd_state", &Machine_State::lock_cmd_state);

  rttr::registration::class_<Hydraulic_Flow_Rate_2>("Hydraulic_Flow_Rate_2")
      .constructor<>()
      .property("Hydraulic_Flow_Rate_2::left_track_motor_b_flow_rate", &Hydraulic_Flow_Rate_2::left_track_motor_b_flow_rate)
      .property("Hydraulic_Flow_Rate_2::left_track_motor_a_flow_rate", &Hydraulic_Flow_Rate_2::left_track_motor_a_flow_rate)
      .property("Hydraulic_Flow_Rate_2::right_track_motor_b_flow_rate", &Hydraulic_Flow_Rate_2::right_track_motor_b_flow_rate)
      .property("Hydraulic_Flow_Rate_2::right_track_motor_a_flow_rate", &Hydraulic_Flow_Rate_2::right_track_motor_a_flow_rate);

  rttr::registration::class_<Hydraulic_Flow_Rate_1>("Hydraulic_Flow_Rate_1")
      .constructor<>()
      .property("Hydraulic_Flow_Rate_1::swing_motor_b_flow_rate", &Hydraulic_Flow_Rate_1::swing_motor_b_flow_rate)
      .property("Hydraulic_Flow_Rate_1::swing_motor_a_flow_rate", &Hydraulic_Flow_Rate_1::swing_motor_a_flow_rate)
      .property("Hydraulic_Flow_Rate_1::bucket_cylinder_rod_flow_rate", &Hydraulic_Flow_Rate_1::bucket_cylinder_rod_flow_rate)
      .property("Hydraulic_Flow_Rate_1::bucket_cylinder_bottom_flow_rate", &Hydraulic_Flow_Rate_1::bucket_cylinder_bottom_flow_rate)
      .property("Hydraulic_Flow_Rate_1::arm_cylinder_rod_flow_rate", &Hydraulic_Flow_Rate_1::arm_cylinder_rod_flow_rate)
      .property("Hydraulic_Flow_Rate_1::arm_cylinder_bottom_flow_rate", &Hydraulic_Flow_Rate_1::arm_cylinder_bottom_flow_rate)
      .property("Hydraulic_Flow_Rate_1::boom_cylinder_rod_flow_rate", &Hydraulic_Flow_Rate_1::boom_cylinder_rod_flow_rate)
      .property("Hydraulic_Flow_Rate_1::boom_cylinder_bottom_flow_rate", &Hydraulic_Flow_Rate_1::boom_cylinder_bottom_flow_rate);

  rttr::registration::class_<Pilot_Pressure_2>("Pilot_Pressure_2")
      .constructor<>()
      .property("Pilot_Pressure_2::assist_b_pilot_pressure", &Pilot_Pressure_2::assist_b_pilot_pressure)
      .property("Pilot_Pressure_2::assist_a_pilot_pressure", &Pilot_Pressure_2::assist_a_pilot_pressure)
      .property("Pilot_Pressure_2::attachment_b_pilot_pressure", &Pilot_Pressure_2::attachment_b_pilot_pressure)
      .property("Pilot_Pressure_2::attachment_a_pilot_pressure", &Pilot_Pressure_2::attachment_a_pilot_pressure)
      .property("Pilot_Pressure_2::left_track_backward_pilot_prs", &Pilot_Pressure_2::left_track_backward_pilot_prs)
      .property("Pilot_Pressure_2::left_track_forward_pilot_prs", &Pilot_Pressure_2::left_track_forward_pilot_prs)
      .property("Pilot_Pressure_2::right_track_backward_pilot_prs", &Pilot_Pressure_2::right_track_backward_pilot_prs)
      .property("Pilot_Pressure_2::right_track_forward_pilot_prs", &Pilot_Pressure_2::right_track_forward_pilot_prs);

  rttr::registration::class_<Pilot_Pressure_1>("Pilot_Pressure_1")
      .constructor<>()
      .property("Pilot_Pressure_1::swing_left_pilot_pressure", &Pilot_Pressure_1::swing_left_pilot_pressure)
      .property("Pilot_Pressure_1::swing_right_pilot_pressure", &Pilot_Pressure_1::swing_right_pilot_pressure)
      .property("Pilot_Pressure_1::bucket_dump_pilot_pressure", &Pilot_Pressure_1::bucket_dump_pilot_pressure)
      .property("Pilot_Pressure_1::bucket_crowd_pilot_pressure", &Pilot_Pressure_1::bucket_crowd_pilot_pressure)
      .property("Pilot_Pressure_1::arm_dump_pilot_pressure", &Pilot_Pressure_1::arm_dump_pilot_pressure)
      .property("Pilot_Pressure_1::arm_crowd_pilot_pressure", &Pilot_Pressure_1::arm_crowd_pilot_pressure)
      .property("Pilot_Pressure_1::boom_down_pilot_pressure", &Pilot_Pressure_1::boom_down_pilot_pressure)
      .property("Pilot_Pressure_1::boom_up_pilot_pressure", &Pilot_Pressure_1::boom_up_pilot_pressure);

  rttr::registration::class_<Pressure_2>("Pressure_2")
      .constructor<>()
      .property("Pressure_2::assit_b_pressure", &Pressure_2::assit_b_pressure)
      .property("Pressure_2::assist_a_pressure", &Pressure_2::assist_a_pressure)
      .property("Pressure_2::attachment_b_pressure", &Pressure_2::attachment_b_pressure)
      .property("Pressure_2::attachment_a_pressure", &Pressure_2::attachment_a_pressure)
      .property("Pressure_2::left_track_motor_b_pressure", &Pressure_2::left_track_motor_b_pressure)
      .property("Pressure_2::left_track_motor_a_pressure", &Pressure_2::left_track_motor_a_pressure)
      .property("Pressure_2::right_track_motor_b_pressure", &Pressure_2::right_track_motor_b_pressure)
      .property("Pressure_2::right_track_motor_a_pressure", &Pressure_2::right_track_motor_a_pressure);

  rttr::registration::class_<Pressure_1>("Pressure_1")
      .constructor<>()
      .property("Pressure_1::swing_motor_b_pressure", &Pressure_1::swing_motor_b_pressure)
      .property("Pressure_1::swing_motor_a_pressure", &Pressure_1::swing_motor_a_pressure)
      .property("Pressure_1::bucket_cylinder_rod_pressure", &Pressure_1::bucket_cylinder_rod_pressure)
      .property("Pressure_1::bucket_cylinder_bottom_pressure", &Pressure_1::bucket_cylinder_bottom_pressure)
      .property("Pressure_1::arm_cylinder_rod_pressure", &Pressure_1::arm_cylinder_rod_pressure)
      .property("Pressure_1::arm_cylinder_bottom_pressure", &Pressure_1::arm_cylinder_bottom_pressure)
      .property("Pressure_1::boom_cylinder_rod_pressure", &Pressure_1::boom_cylinder_rod_pressure)
      .property("Pressure_1::boom_cylinder_bottom_pressure", &Pressure_1::boom_cylinder_bottom_pressure);

  rttr::registration::class_<Vehicle_Azimuth>("Vehicle_Azimuth")
      .constructor<>()
      .property("Vehicle_Azimuth::track_body_azimuth", &Vehicle_Azimuth::track_body_azimuth)
      .property("Vehicle_Azimuth::upper_body_azimuth", &Vehicle_Azimuth::upper_body_azimuth);

  rttr::registration::class_<Swing_Center_Position_3>("Swing_Center_Position_3")
      .constructor<>()
      .property("Swing_Center_Position_3::swing_center_position_z", &Swing_Center_Position_3::swing_center_position_z);

  rttr::registration::class_<Swing_Center_Position_2>("Swing_Center_Position_2")
      .constructor<>()
      .property("Swing_Center_Position_2::swing_center_position_y", &Swing_Center_Position_2::swing_center_position_y);

  rttr::registration::class_<Swing_Center_Position_1>("Swing_Center_Position_1")
      .constructor<>()
      .property("Swing_Center_Position_1::swing_center_position_x", &Swing_Center_Position_1::swing_center_position_x);

  rttr::registration::class_<Front_Pin_Position_3>("Front_Pin_Position_3")
      .constructor<>()
      .property("Front_Pin_Position_3::bucket_pin_position_z", &Front_Pin_Position_3::bucket_pin_position_z)
      .property("Front_Pin_Position_3::bucket_pin_position_y", &Front_Pin_Position_3::bucket_pin_position_y)
      .property("Front_Pin_Position_3::bucket_pin_position_x", &Front_Pin_Position_3::bucket_pin_position_x);

  rttr::registration::class_<Front_Pin_Position_2>("Front_Pin_Position_2")
      .constructor<>()
      .property("Front_Pin_Position_2::arm_pin_position_z", &Front_Pin_Position_2::arm_pin_position_z)
      .property("Front_Pin_Position_2::arm_pin_position_y", &Front_Pin_Position_2::arm_pin_position_y)
      .property("Front_Pin_Position_2::arm_pin_position_x", &Front_Pin_Position_2::arm_pin_position_x);

  rttr::registration::class_<Front_Pin_Position_1>("Front_Pin_Position_1")
      .constructor<>()
      .property("Front_Pin_Position_1::boom_pin_position_z", &Front_Pin_Position_1::boom_pin_position_z)
      .property("Front_Pin_Position_1::boom_pin_position_y", &Front_Pin_Position_1::boom_pin_position_y)
      .property("Front_Pin_Position_1::boom_pin_position_x", &Front_Pin_Position_1::boom_pin_position_x);

  rttr::registration::class_<Roll_Pitch_Angle>("Roll_Pitch_Angle")
      .constructor<>()
      .property("Roll_Pitch_Angle::pitch_angle", &Roll_Pitch_Angle::pitch_angle)
      .property("Roll_Pitch_Angle::roll_angle", &Roll_Pitch_Angle::roll_angle);

  rttr::registration::class_<Front_Angular_Velocity>("Front_Angular_Velocity")
      .constructor<>()
      .property("Front_Angular_Velocity::swing_relative_angular_velocity", &Front_Angular_Velocity::swing_relative_angular_velocity)
      .property("Front_Angular_Velocity::bucket_relative_angular_velocity", &Front_Angular_Velocity::bucket_relative_angular_velocity)
      .property("Front_Angular_Velocity::arm_relative_angular_velocity", &Front_Angular_Velocity::arm_relative_angular_velocity)
      .property("Front_Angular_Velocity::boom_relative_angular_velocity", &Front_Angular_Velocity::boom_relative_angular_velocity);

  rttr::registration::class_<Front_Angle>("Front_Angle")
      .constructor<>()
      .property("Front_Angle::bucket_relative_angle", &Front_Angle::bucket_relative_angle)
      .property("Front_Angle::swing_relative_angle", &Front_Angle::swing_relative_angle)
      .property("Front_Angle::arm_relative_angle", &Front_Angle::arm_relative_angle)
      .property("Front_Angle::boom_relative_angle", &Front_Angle::boom_relative_angle);

  rttr::registration::class_<Machine_Setting_Cmd>("Machine_Setting_Cmd")
      .constructor<>()
      .property("Machine_Setting_Cmd::working_mode_notice", &Machine_Setting_Cmd::working_mode_notice)
      .property("Machine_Setting_Cmd::yellow_led_mode", &Machine_Setting_Cmd::yellow_led_mode)
      .property("Machine_Setting_Cmd::travel_speed_mode", &Machine_Setting_Cmd::travel_speed_mode)
      .property("Machine_Setting_Cmd::power_eco_mode", &Machine_Setting_Cmd::power_eco_mode)
      .property("Machine_Setting_Cmd::alive_counter", &Machine_Setting_Cmd::alive_counter)
      .property("Machine_Setting_Cmd::engine_rpm", &Machine_Setting_Cmd::engine_rpm);

  rttr::registration::class_<Pilot_Pressure_Cmd_2>("Pilot_Pressure_Cmd_2")
      .constructor<>()
      .property("Pilot_Pressure_Cmd_2::assist_b", &Pilot_Pressure_Cmd_2::assist_b)
      .property("Pilot_Pressure_Cmd_2::assist_a", &Pilot_Pressure_Cmd_2::assist_a)
      .property("Pilot_Pressure_Cmd_2::attachment_b", &Pilot_Pressure_Cmd_2::attachment_b)
      .property("Pilot_Pressure_Cmd_2::attachment_a", &Pilot_Pressure_Cmd_2::attachment_a)
      .property("Pilot_Pressure_Cmd_2::left_track_backward", &Pilot_Pressure_Cmd_2::left_track_backward)
      .property("Pilot_Pressure_Cmd_2::right_track_backward", &Pilot_Pressure_Cmd_2::right_track_backward)
      .property("Pilot_Pressure_Cmd_2::left_track_forward", &Pilot_Pressure_Cmd_2::left_track_forward)
      .property("Pilot_Pressure_Cmd_2::right_track_forward", &Pilot_Pressure_Cmd_2::right_track_forward);

  rttr::registration::class_<Pilot_Pressure_Cmd_1>("Pilot_Pressure_Cmd_1")
      .constructor<>()
      .property("Pilot_Pressure_Cmd_1::swing_right", &Pilot_Pressure_Cmd_1::swing_right)
      .property("Pilot_Pressure_Cmd_1::swing_left", &Pilot_Pressure_Cmd_1::swing_left)
      .property("Pilot_Pressure_Cmd_1::bucket_dump", &Pilot_Pressure_Cmd_1::bucket_dump)
      .property("Pilot_Pressure_Cmd_1::bucket_crowd", &Pilot_Pressure_Cmd_1::bucket_crowd)
      .property("Pilot_Pressure_Cmd_1::arm_dump", &Pilot_Pressure_Cmd_1::arm_dump)
      .property("Pilot_Pressure_Cmd_1::arm_crowd", &Pilot_Pressure_Cmd_1::arm_crowd)
      .property("Pilot_Pressure_Cmd_1::boom_down", &Pilot_Pressure_Cmd_1::boom_down)
      .property("Pilot_Pressure_Cmd_1::boom_up", &Pilot_Pressure_Cmd_1::boom_up);

  rttr::registration::class_<Can_Bus>("Can_Bus")
      .constructor<>();
}