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

struct Lever_cmd
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

class can_handler
{
  public:
    can_handler(boost::asio::io_context &io, std::string can_port)
        : send_timer(io, boost::asio::chrono::milliseconds(cmd_interval)),
          send_timer1(io, boost::asio::chrono::milliseconds(cmd1_interval)),
          send_timer2(io, boost::asio::chrono::milliseconds(cmd2_interval)),
          sock(io)
    {
      const auto idx = canary::get_interface_index(can_port);
      auto const ep = canary::raw::endpoint{idx};
      sock.open();
      sock.bind(ep);

      // l_cmd = Lever_cmd{};
      l_cmd2 = boost::shared_ptr<Lever_cmd>(new Lever_cmd{});

      std::cout << "shared_ptr:" << l_cmd2->swing_right << std::endl;

      // l_cmd[0].
      start_receive();
      send_timer.async_wait(boost::bind(&can_handler::send_cmd, this));
      send_timer1.async_wait(boost::bind(&can_handler::send_cmd1, this));
      send_timer2.async_wait(boost::bind(&can_handler::send_cmd2, this));
    }
    void send_cmd()
    {
        std::cout << "Test" << std::endl;

        frame f = {};
        f.header.id(0x12345678);
        f.header.extended_format(true);
        f.header.payload_length(8);
        f.payload[0] = l_cmd2->boom_raise;
        f.payload[1] = l_cmd2->boom_lower;
        f.payload[2] = l_cmd2->arm_dump;
        f.payload[3] = l_cmd2->arm_crowd;
        f.payload[4] = l_cmd2->bucket_dump;
        f.payload[5] = l_cmd2->bucket_crowd;
        f.payload[6] = l_cmd2->swing_left;
        f.payload[7] = l_cmd2->swing_right;

        // sock.send(canary::net::buffer(&f, sizeof(f)));
        sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::nullhandle,this));
        send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(cmd_interval));
        send_timer.async_wait(boost::bind(&can_handler::send_cmd, this));
    }
    void send_cmd1()
    {
      std::cout << "Test1" << std::endl;

      frame f = {};
      f.header.id(0x1234567A);
      f.header.extended_format(true);
      f.header.payload_length(8);

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::nullhandle, this));
      send_timer1.expires_at(send_timer1.expiry() + boost::asio::chrono::milliseconds(cmd1_interval));
      send_timer1.async_wait(boost::bind(&can_handler::send_cmd1, this));
    }
    void send_cmd2()
    {
      std::cout << "Test2" << std::endl;
      
      ///
      //boost share_ptr入れる？？？
      ///

      frame f = {};
      f.header.id(0x1234567B);
      f.header.extended_format(true);
      f.header.payload_length(8);

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::nullhandle, this));
      send_timer2.expires_at(send_timer2.expiry() + boost::asio::chrono::milliseconds(cmd2_interval));
      send_timer2.async_wait(boost::bind(&can_handler::send_cmd2, this));
    }
    void lever_cmd(Lever_cmd cmd)
    {
      //l_cmd = cmd;
      l_cmd2 = boost::make_shared<Lever_cmd>(cmd);
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
      // Lever_cmd l_cmd;
      boost::shared_ptr<Lever_cmd> l_cmd2;
    };

int main(int argc, char **argv)
{
  //io setting
  canary::net::io_context ioc;

  can_handler h(ioc,"vcan0");

  // ioc.run();
  boost::thread t(boost::bind(&boost::asio::io_context::run, &ioc));
  while(true){
    sleep(1);
    // Lever_cmd cmd = Lever_cmd{};
    // cmd.boom_raise = 100;
    // h.lever_cmd(cmd);
    std::cout << "main loop" << std::endl;
  }
}