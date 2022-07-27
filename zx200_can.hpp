#ifndef ZX200_CAN_HPP
#define ZX200_CAN_HPP

#include <iostream>

// send data from PC to ZX200

struct pilot_pressure_cmd1
{
  std::uint8_t boom_raise;
  std::uint8_t boom_lower;
  std::uint8_t arm_dump;
  std::uint8_t arm_crowd;
  std::uint8_t bucket_dump;
  std::uint8_t bucket_crowd;
  std::uint8_t swing_left;
  std::uint8_t swing_right;
};

struct pilot_pressure_cmd2
{
  std::uint8_t track_right_forward;
  std::uint8_t track_left_forward;
  std::uint8_t track_right_backward;
  std::uint8_t track_left_backward;
  std::uint8_t spare_A;
  std::uint8_t spare_B;
  std::uint8_t assist_A;
  std::uint8_t assist_B;
};

struct machine_setting_cmd
{
  std::uint8_t engine_rpm;
  std::uint8_t power_eco_mode;
  std::uint8_t rabbit_turtle_mode;
  std::uint8_t status_notice;
  std::uint8_t yellow_lamp_cmd;
  // byte3-7 is not assigned
  // std::uint8_t alive_cnt;
};

// receive data from ZX200 to PC

struct front_angle
{
  double boom_angle;
  double arm_angle;
  double bucket_angle;
  double swing_angle;
};

struct front_ang_velocity
{
  double boom_ang_velocity;
  double arm_ang_velocity;
  double bucket_ang_velocity;
  double swing_ang_velocity;
};

struct roll_pitch_angle
{
  double roll_angle;
  double pitch_angle;
};

#endif //ZX200_CAN_HPP