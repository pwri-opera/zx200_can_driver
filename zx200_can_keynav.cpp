#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>
#include <canary/socket_options.hpp>
#include <iostream>
#include <thread>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>

#include <boost/bind.hpp>

#include <ncurses.h>

#define cmd_interval 1000
#define cmd1_interval 1000
#define cmd2_interval 1000

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

class can_handler
{
  public:
    can_handler(boost::asio::io_context &io, std::string can_port)
        : send_timer(io, boost::asio::chrono::milliseconds(cmd_interval)),
          send_timer1(io, boost::asio::chrono::milliseconds(cmd1_interval)),
          send_timer2(io, boost::asio::chrono::milliseconds(cmd2_interval)),
          sock(io), alive_cnt(0)
    {
      const auto idx = canary::get_interface_index(can_port);
      auto const ep = canary::raw::endpoint{idx};
      sock.open();
      sock.bind(ep);

      pi_cmd1 = boost::shared_ptr<pilot_pressure_cmd1>(new pilot_pressure_cmd1{});
      pi_cmd2 = boost::shared_ptr<pilot_pressure_cmd2>(new pilot_pressure_cmd2{});
      setting_cmd = boost::shared_ptr<machine_setting_cmd>(new machine_setting_cmd{});

      start_receive();
      send_timer.async_wait(boost::bind(&can_handler::send_pi_cmd1, this));
      send_timer1.async_wait(boost::bind(&can_handler::send_pi_cmd2, this));
      send_timer2.async_wait(boost::bind(&can_handler::send_machine_setting_cmd, this));
    }

    void send_pi_cmd1()
    {
        std::cout << "Test" << std::endl;

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
        sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::nullhandle,this));
        send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(cmd_interval));
        send_timer.async_wait(boost::bind(&can_handler::send_pi_cmd1, this));
    }

    void send_pi_cmd2()
    {
      std::cout << "Test1" << std::endl;

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
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::nullhandle, this));
      send_timer1.expires_at(send_timer1.expiry() + boost::asio::chrono::milliseconds(cmd1_interval));
      send_timer1.async_wait(boost::bind(&can_handler::send_pi_cmd2, this));
    }

    void send_machine_setting_cmd()
    {
      std::cout << "Test2" << std::endl;

      frame f = {};
      f.header.id(0x18FF660A);
      f.header.extended_format(true);
      f.header.payload_length(8);
      f.payload[0] = setting_cmd->engine_rpm;
      f.payload[1] = std::uint8_t(
          setting_cmd->yellow_lamp_cmd << 4 | setting_cmd->status_notice << 3 |setting_cmd->rabbit_turtle_mode << 2 | setting_cmd->power_eco_mode);
      f.payload[7] = alive_cnt++;

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::nullhandle, this));
      send_timer2.expires_at(send_timer2.expiry() + boost::asio::chrono::milliseconds(cmd2_interval));
      send_timer2.async_wait(boost::bind(&can_handler::send_machine_setting_cmd, this));
    }
    void set_pilot_pressure_cmd1(pilot_pressure_cmd1 cmd)
    {
      pi_cmd1 = boost::make_shared<pilot_pressure_cmd1>(cmd);
    }

    private:
      void nullhandle(){};
      void start_receive()
      {
        frame f = {};
        sock.async_receive(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::handle_receive, this, boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,f));
      }
      void handle_receive(const boost::system::error_code &error,
                          std::size_t /*bytes_transferred*/,frame f)
      {
        if (!error)
        {
          // boost::shared_ptr<frame> fr(f);
          std::cout
              << "Received CAN frame, id: " << std::hex
              << f.header.id() << " len: " << std::dec
              << f.header.payload_length() << '\n';
          start_receive();
        }
      }
      boost::asio::steady_timer send_timer, send_timer1, send_timer2;
      canary::raw::socket sock;
      boost::shared_ptr<pilot_pressure_cmd1> pi_cmd1;
      boost::shared_ptr<pilot_pressure_cmd2> pi_cmd2;
      boost::shared_ptr<machine_setting_cmd> setting_cmd;
      std::uint8_t alive_cnt;
};



int main(int argc, char **argv)
{
  canary::net::io_context ioc;
  can_handler h(ioc,"vcan0");

  boost::thread t(boost::bind(&boost::asio::io_context::run, &ioc));
  while(true){
    sleep(1);
    std::cout << "main loop" << std::endl;
  }
}