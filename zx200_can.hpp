#pragma once

#include <iostream>
#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>

#include <dbcppp/Network.h>
#include <dbcppp/Network2Functions.h>
#include <fstream>

#include "zx200_dbc.hpp"

// receive data from ZX200 to PC


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


class zx200_can:zx200_dbc
{
  public:
    zx200_can(boost::asio::io_context &io, std::string can_port)
        : send_timer(io, boost::asio::chrono::milliseconds(pi_cmd1_interval)),
          send_timer1(io, boost::asio::chrono::milliseconds(pi_cmd2_interval)),
          send_timer2(io, boost::asio::chrono::milliseconds(setting_cmd_interval)),
          sock(io)
    {
      const auto idx = canary::get_interface_index(can_port);
      auto const ep = canary::raw::endpoint{idx};
      sock.open();
      sock.bind(ep);

      // pi_cmd1 = boost::shared_ptr<pilot_pressure_cmd1>(new pilot_pressure_cmd1{});
      pi_cmd2 = boost::shared_ptr<pilot_pressure_cmd2>(new pilot_pressure_cmd2{});
      setting_cmd = boost::shared_ptr<machine_setting_cmd>(new machine_setting_cmd{});

      // create_dbc_map(pi_cmd1);

      start_receive();
      send_timer.async_wait(boost::bind(&zx200_can::send_pi_cmd1, this));
      send_timer1.async_wait(boost::bind(&zx200_can::send_pi_cmd2, this));
      send_timer2.async_wait(boost::bind(&zx200_can::send_machine_setting_cmd, this));
    };
    ~zx200_can()
    {
      send_timer.cancel();
      send_timer1.cancel();
      send_timer2.cancel();
    }

    void set_pilot_pressure_cmd1(pilot_pressure_cmd1 cmd)
    {
      // pi_cmd1.at("boom_up") = cmd.boom_up;
      // pi_cmd1.at("boom_down") = cmd.boom_down;
      // pi_cmd1.at("arm_crowd") = cmd.arm_crowd;
      // pi_cmd1.at("arm_dump") = cmd.arm_dump;
      // pi_cmd1.at("bucket_crowd") = cmd.bucket_crowd;
      // pi_cmd1.at("bucket_dump") = cmd.bucket_dump;
      // pi_cmd1.at("swing_right") = cmd.swing_right;
      // pi_cmd1.at("swing_left") = cmd.swing_left;
      pi_cmd1_new = cmd;
    }
    void set_pilot_pressure_cmd2(pilot_pressure_cmd2 cmd)
    {
      pi_cmd2 = boost::make_shared<pilot_pressure_cmd2>(cmd);
    }
    void set_machine_setting_cmd(machine_setting_cmd cmd)
    {
      // setting_cmd = boost::make_shared<machine_setting_cmd>(cmd);
      setting_cmdx.at("engine_rpm") = cmd.engine_rpm;
      setting_cmdx.at("power_eco_mode") = cmd.power_eco_mode;
      setting_cmdx.at("travel_speed_mode") = cmd.travel_speed_mode;
      setting_cmdx.at("working_mode_notice") = cmd.working_mode_notice;
      setting_cmdx.at("yellow_led_mode") = cmd.yellow_led_mode;
      setting_cmdx.at("alive_counter") = cmd.alive_counter;
    }

  private:
    void send_pi_cmd1()
    {
      frame f;
      // pi_cmd1_encode(pi_cmd1, f);
      pi_cmd1_encode(pi_cmd1_new, f);

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(pi_cmd1_interval));
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
      f.payload[4] = pi_cmd2->assist_a;
      f.payload[5] = pi_cmd2->attachment_b;
      f.payload[6] = pi_cmd2->assist_a;
      f.payload[7] = pi_cmd2->assist_b;

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer1.expires_at(send_timer1.expiry() + boost::asio::chrono::milliseconds(pi_cmd2_interval));
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
          setting_cmd->yellow_led_mode << 4 | setting_cmd->working_mode_notice << 3 | setting_cmd->travel_speed_mode << 2 | setting_cmd->power_eco_mode);
      f.payload[7] = alive_cnt++;

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer2.expires_at(send_timer2.expiry() + boost::asio::chrono::milliseconds(setting_cmd_interval));
      send_timer2.async_wait(boost::bind(&zx200_can::send_machine_setting_cmd, this));
    }

    void send_handle() {}

    void start_receive()
    {
      sock.async_receive(canary::net::buffer(&recv_f, sizeof(recv_f)), boost::bind(&zx200_can::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

      if (recv_f.header.payload_length())
      {
        // wprintw(gwUI, "Received CAN frame, id: %x, len: %ld\n", recv_f.header.id(), recv_f.header.payload_length());
        //decode here
        decode(recv_f);
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
    // boost::shared_ptr<pilot_pressure_cmd1> pi_cmd1;
    pilot_pressure_cmd1 pi_cmd1_new;
    boost::shared_ptr<pilot_pressure_cmd2> pi_cmd2;
    boost::shared_ptr<machine_setting_cmd> setting_cmd;
    std::uint8_t alive_cnt;
    std::unordered_map<uint64_t, const dbcppp::IMessage *> messages;
    std::map<std::string, double> pi_cmd1,pi_cmd2x;
    std::map<std::string, int> setting_cmdx;
};