#pragma once

#include <iostream>
#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>

#define cmd_interval 10
#define cmd1_interval 10
#define cmd2_interval 50

struct frame
{
  canary::frame_header header;
  std::array<std::uint8_t, 8> payload;
};

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


class zx200_can
{
  public:
    zx200_can(boost::asio::io_context &io, std::string can_port): send_timer(io, boost::asio::chrono::milliseconds(cmd_interval)),
    send_timer1(io, boost::asio::chrono::milliseconds(cmd1_interval)),
    send_timer2(io, boost::asio::chrono::milliseconds(cmd2_interval)),
    sock(io)
    {
      const auto idx = canary::get_interface_index(can_port);
      auto const ep = canary::raw::endpoint{idx};
      sock.open();
      sock.bind(ep);

      pi_cmd1 = boost::shared_ptr<pilot_pressure_cmd1>(new pilot_pressure_cmd1{});
      pi_cmd2 = boost::shared_ptr<pilot_pressure_cmd2>(new pilot_pressure_cmd2{});
      setting_cmd = boost::shared_ptr<machine_setting_cmd>(new machine_setting_cmd{});

      start_receive();
      send_timer.async_wait(boost::bind(&zx200_can::send_pi_cmd1, this));
      send_timer1.async_wait(boost::bind(&zx200_can::send_pi_cmd2, this));
      send_timer2.async_wait(boost::bind(&zx200_can::send_machine_setting_cmd, this));
    };


    void set_pilot_pressure_cmd1(pilot_pressure_cmd1 cmd)
    {
      pi_cmd1 = boost::make_shared<pilot_pressure_cmd1>(cmd);
    }
    void set_pilot_pressure_cmd2(pilot_pressure_cmd2 cmd)
    {
      pi_cmd2 = boost::make_shared<pilot_pressure_cmd2>(cmd);
    }
    void set_machine_setting_cmd(machine_setting_cmd cmd)
    {
      setting_cmd = boost::make_shared<machine_setting_cmd>(cmd);
    }

    // void set_pilot_pressure_cmd1a(boost::make_shared<pilot_pressure_cmd1> cmd)
    // {
    // }
    // void set_pilot_pressure_cmd2(pilot_pressure_cmd2 cmd)
    // {
    //   pi_cmd2 = boost::make_shared<pilot_pressure_cmd2>(cmd);
    // }
    // void set_machine_setting_cmd(machine_setting_cmd cmd)
    // {
    //   setting_cmd = boost::make_shared<machine_setting_cmd>(cmd);
    // }

  private:
    void send_pi_cmd1()
    {
      frame f = {};
      f.header.id(0x18FF640A);
      f.header.extended_format(true);
      f.header.payload_length(8);
      f.payload[0] = pi_cmd1->boom_raise;
      f.payload[1] = pi_cmd1->boom_lower;
      f.payload[2] = pi_cmd1->arm_dump;
      f.payload[3] = pi_cmd1->arm_crowd;
      f.payload[4] = pi_cmd1->bucket_dump;
      f.payload[5] = pi_cmd1->bucket_crowd;
      f.payload[6] = pi_cmd1->swing_left;
      f.payload[7] = pi_cmd1->swing_right;

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(cmd_interval));
      send_timer.async_wait(boost::bind(&zx200_can::send_pi_cmd1, this));
    }

    void send_pi_cmd2()
    {
      frame f = {};
      f.header.id(0x18FF650A);
      f.header.extended_format(true);
      f.header.payload_length(8);
      f.payload[0] = pi_cmd2->track_right_forward;
      f.payload[1] = pi_cmd2->track_left_forward;
      f.payload[2] = pi_cmd2->track_right_backward;
      f.payload[3] = pi_cmd2->track_left_backward;
      f.payload[4] = pi_cmd2->spare_A;
      f.payload[5] = pi_cmd2->spare_B;
      f.payload[6] = pi_cmd2->assist_A;
      f.payload[7] = pi_cmd2->assist_B;

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer1.expires_at(send_timer1.expiry() + boost::asio::chrono::milliseconds(cmd1_interval));
      send_timer1.async_wait(boost::bind(&zx200_can::send_pi_cmd2, this));
    }

    void send_machine_setting_cmd()
    {
      frame f = {};
      f.header.id(0x18FF660A);
      f.header.extended_format(true);
      f.header.payload_length(8);
      f.payload[0] = setting_cmd->engine_rpm;
      f.payload[1] = std::uint8_t(
          setting_cmd->yellow_lamp_cmd << 4 | setting_cmd->status_notice << 3 | setting_cmd->rabbit_turtle_mode << 2 | setting_cmd->power_eco_mode);
      f.payload[7] = alive_cnt++;

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer2.expires_at(send_timer2.expiry() + boost::asio::chrono::milliseconds(cmd2_interval));
      send_timer2.async_wait(boost::bind(&zx200_can::send_machine_setting_cmd, this));
    }

    void send_handle() {}

    void start_receive()
    {
      sock.async_receive(canary::net::buffer(&recv_f, sizeof(recv_f)), boost::bind(&zx200_can::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

      if (recv_f.header.payload_length())
      {
        // wprintw(gwUI, "Received CAN frame, id: %x, len: %ld\n", recv_f.header.id(), recv_f.header.payload_length());
      }
    }

    void handle_receive(const boost::system::error_code &error,
                        std::size_t /*bytes_transferred*/)
    {
      if (!error)
      {
        start_receive();
      }
    }

    boost::asio::steady_timer send_timer, send_timer1, send_timer2;
    canary::raw::socket sock;
    frame recv_f;
    // pilot_pressure_cmd1 cmd1;
    boost::shared_ptr<pilot_pressure_cmd1> pi_cmd1;
    boost::shared_ptr<pilot_pressure_cmd2> pi_cmd2;
    boost::shared_ptr<machine_setting_cmd> setting_cmd;
    std::uint8_t alive_cnt;
};
// #endif //ZX200_CAN_HPP