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

class zx200_can:zx200_dbc
{
  public:
    zx200_can(boost::asio::io_context &io, std::string can_port)
        : send_timer(io, boost::asio::chrono::milliseconds(initial_interval)),
          send_timer1(io, boost::asio::chrono::milliseconds(initial_interval)),
          send_timer2(io, boost::asio::chrono::milliseconds(initial_interval)),
          sock(io)
    {
      const auto idx = canary::get_interface_index(can_port);
      auto const ep = canary::raw::endpoint{idx};
      sock.open();
      sock.bind(ep);

      pi_cmd1 = boost::shared_ptr<Pilot_Pressure_Cmd_1>(new Pilot_Pressure_Cmd_1{});
      pi_cmd2 = boost::shared_ptr<Pilot_Pressure_Cmd_2>(new Pilot_Pressure_Cmd_2{});
      setting_cmd = boost::shared_ptr<Machine_Setting_Cmd>(new Machine_Setting_Cmd{});

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

    void set_pilot_pressure_cmd1(Pilot_Pressure_Cmd_1 cmd)
    {
      pi_cmd1 = boost::make_shared<Pilot_Pressure_Cmd_1>(cmd);
    }
    void set_pilot_pressure_cmd2(Pilot_Pressure_Cmd_2 cmd)
    {
      pi_cmd2 = boost::make_shared<Pilot_Pressure_Cmd_2>(cmd);
    }
    void set_machine_setting_cmd(Machine_Setting_Cmd cmd)
    {
      setting_cmd = boost::make_shared<Machine_Setting_Cmd>(cmd);
    }

  private:
    void send_pi_cmd1()
    {
      frame f;
      encode(*pi_cmd1,f);

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(pi_cmd1->Cycle_time()));
      send_timer.async_wait(boost::bind(&zx200_can::send_pi_cmd1, this));
    }

    void send_pi_cmd2()
    {
      frame f;
      encode(*pi_cmd2, f);

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&zx200_can::send_handle, this));
      send_timer1.expires_at(send_timer1.expiry() + boost::asio::chrono::milliseconds(pi_cmd2->Cycle_time()));
      send_timer1.async_wait(boost::bind(&zx200_can::send_pi_cmd2, this));
    }

    void send_machine_setting_cmd()
    {
      frame f;
      setting_cmd->alive_counter++;
      encode(*setting_cmd, f);

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
    boost::shared_ptr<Pilot_Pressure_Cmd_1> pi_cmd1;
    boost::shared_ptr<Pilot_Pressure_Cmd_2> pi_cmd2;
    boost::shared_ptr<Machine_Setting_Cmd> setting_cmd;
    std::uint8_t alive_cnt;
    std::unordered_map<uint64_t, const dbcppp::IMessage *> messages;
};