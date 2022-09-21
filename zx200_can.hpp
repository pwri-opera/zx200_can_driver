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
#include <boost/timer/timer.hpp>

#define pi_cmd1_interval 10
#define pi_cmd2_interval 10
#define setting_cmd_interval 50

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

      pi_cmd1 = boost::shared_ptr<pilot_pressure_cmd1>(new pilot_pressure_cmd1{});
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

  private:
    void send_pi_cmd1()
    {
      frame f;
      encode(2566874122,*pi_cmd1,f);

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(pi_cmd1_interval));
      send_timer.async_wait(boost::bind(&zx200_can::send_pi_cmd1, this));
    }

    void send_pi_cmd2()
    {
      frame f;
      encode(2566874378, *pi_cmd2, f);

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer1.expires_at(send_timer1.expiry() + boost::asio::chrono::milliseconds(pi_cmd2_interval));
      send_timer1.async_wait(boost::bind(&zx200_can::send_pi_cmd2, this));
    }

    void send_machine_setting_cmd()
    {
      frame f;
      setting_cmd->alive_counter++;
      encode(2566874634, *setting_cmd, f);

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
    boost::shared_ptr<pilot_pressure_cmd1> pi_cmd1;
    boost::shared_ptr<pilot_pressure_cmd2> pi_cmd2;
    boost::shared_ptr<machine_setting_cmd> setting_cmd;
    std::uint8_t alive_cnt;
    std::unordered_map<uint64_t, const dbcppp::IMessage *> messages;
};