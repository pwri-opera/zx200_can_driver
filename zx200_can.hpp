#pragma once

#include <iostream>
#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>

#include <dbcppp/Network.h>
#include <dbcppp/Network2Functions.h>
#include <fstream>
#include "zx200_dbc.hpp"

#define initial_interval 10

class zx200_can : public zx200_dbc
{
public:
  zx200_can(boost::asio::io_context &io, std::string can_port, std::string dbc_path)
      : zx200_dbc(dbc_path),
        send_timer(io, boost::asio::chrono::milliseconds(initial_interval)),
        send_timer1(io, boost::asio::chrono::milliseconds(initial_interval)),
        send_timer2(io, boost::asio::chrono::milliseconds(initial_interval)),
        sock(io)
  {
    const auto idx = canary::get_interface_index(can_port);
    auto const ep = canary::raw::endpoint{idx};
    sock.open();
    sock.bind(ep);

    pi_cmd1 = boost::shared_ptr<zx200::Pilot_Pressure_Cmd_1>(new zx200::Pilot_Pressure_Cmd_1{});
    pi_cmd2 = boost::shared_ptr<zx200::Pilot_Pressure_Cmd_2>(new zx200::Pilot_Pressure_Cmd_2{});
    vel_cmd1 = boost::shared_ptr<zx200::Velocity_Cmd_1>(new zx200::Velocity_Cmd_1{});
    vel_cmd2 = boost::shared_ptr<zx200::Velocity_Cmd_2>(new zx200::Velocity_Cmd_2{});
    setting_cmd = boost::shared_ptr<zx200::Machine_Setting_Cmd>(new zx200::Machine_Setting_Cmd{});

    alive_cnt = 0;
    front_control_mode = control_type::None;
    travel_control_mode = control_type::None;
    start_receive();
    send_timer.async_wait(boost::bind(&zx200_can::send_cmd1, this));
    send_timer1.async_wait(boost::bind(&zx200_can::send_cmd2, this));
    send_timer2.async_wait(boost::bind(&zx200_can::send_machine_setting_cmd, this));
    };
    ~zx200_can()
    {
      send_timer.cancel();
      send_timer1.cancel();
      send_timer2.cancel();
    }

    void set_pilot_pressure_cmd1(zx200::Pilot_Pressure_Cmd_1 cmd)
    {
      pi_cmd1 = boost::make_shared<zx200::Pilot_Pressure_Cmd_1>(cmd);
    }
    void set_pilot_pressure_cmd2(zx200::Pilot_Pressure_Cmd_2 cmd)
    {
      pi_cmd2 = boost::make_shared<zx200::Pilot_Pressure_Cmd_2>(cmd);
    }
    void set_machine_setting_cmd(zx200::Machine_Setting_Cmd cmd)
    {
      setting_cmd = boost::make_shared<zx200::Machine_Setting_Cmd>(cmd);
    }

    enum control_type {None=0, Effort=1, Velocity=2, Position=3};

  private:
    void send_cmd1()
    {
      frame f;

      switch(front_control_mode){
        case control_type::None:
          break;
        case control_type::Effort:
          zx200_dbc::encode(*pi_cmd1, f);
          sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
          send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(pi_cmd1->Cycle_time()));
          break;
        case control_type::Velocity:
          zx200_dbc::encode(*vel_cmd1, f);
          sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
          send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(vel_cmd1->Cycle_time()));
          break;
        case control_type::Position:
          break;
      }
      send_timer.async_wait(boost::bind(&zx200_can::send_cmd1, this));
    }

    void send_cmd2()
    {
      frame f;

      switch(travel_control_mode){
        case control_type::None:
          break;
        case control_type::Effort:
          zx200_dbc::encode(*pi_cmd2, f);
          sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
          send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(pi_cmd2->Cycle_time()));
          break;
        case control_type::Velocity:
          zx200_dbc::encode(*vel_cmd2, f);
          sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
          send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(vel_cmd2->Cycle_time()));
          break;
        case control_type::Position:
          break;
      }
      send_timer.async_wait(boost::bind(&zx200_can::send_cmd2, this));

    }

    void send_machine_setting_cmd()
    {
      frame f;
      setting_cmd->alive_counter = alive_cnt++;
      zx200_dbc::encode(*setting_cmd, f);

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer2.expires_at(send_timer2.expiry() + boost::asio::chrono::milliseconds(setting_cmd->Cycle_time()));
      send_timer2.async_wait(boost::bind(&zx200_can::send_machine_setting_cmd, this));
    }

    void send_handle() {}

    void start_receive()
    {
      sock.async_receive(canary::net::buffer(&recv_f, sizeof(recv_f)), boost::bind(&zx200_can::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

      if (recv_f.header.payload_length())
      {
        zx200_dbc::decode(recv_f);
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
    boost::shared_ptr<zx200::Pilot_Pressure_Cmd_1> pi_cmd1;
    boost::shared_ptr<zx200::Pilot_Pressure_Cmd_2> pi_cmd2;
    boost::shared_ptr<zx200::Velocity_Cmd_1> vel_cmd1;
    boost::shared_ptr<zx200::Velocity_Cmd_2> vel_cmd2;
    boost::shared_ptr<zx200::Machine_Setting_Cmd> setting_cmd;
    std::uint8_t alive_cnt,travel_control_mode, front_control_mode;
};