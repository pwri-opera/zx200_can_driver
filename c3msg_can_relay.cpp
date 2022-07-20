#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>
#include <canary/socket_options.hpp>
#include <iostream>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>


int main(int argc, char **argv)
{
  canary::net::io_context ioc;

  const auto idx = canary::get_interface_index("vcan0");
  auto const ep = canary::raw::endpoint{idx};
  canary::raw::socket sock{ioc, ep};

  const auto idx2 = canary::get_interface_index("vcan1");
  auto const ep2 = canary::raw::endpoint{idx2};
  canary::raw::socket sock2{ioc, ep2};

  struct frame
  {
    canary::frame_header header;
    std::array<std::uint8_t, 8> payload;
  };


  canary::net::co_spawn(
      ioc,
      [&sock,&sock2]() -> canary::net::awaitable<void>
      {
        while (true)
        {
          frame f={};
          co_await sock.async_receive(canary::net::buffer(&f, sizeof(f)), canary::net::use_awaitable);

          std::cout << "sock Received CAN frame, id: " << std::hex
                    << f.header.id() << " len: " << std::dec
                    << f.header.payload_length() << '\n';

          co_await sock2.async_send(canary::net::buffer(&f, sizeof(f)), canary::net::use_awaitable);
        }

      },
      canary::net::detached);

  canary::net::co_spawn(
      ioc,
      [&sock, &sock2]() -> canary::net::awaitable<void>
      {
        while (true)
        {
          frame f = {};
          co_await sock2.async_receive(canary::net::buffer(&f, sizeof(f)), canary::net::use_awaitable);

          std::cout << "sock2 Received CAN frame, id: " << std::hex
                    << f.header.id() << " len: " << std::dec
                    << f.header.payload_length() << '\n';

          co_await sock.async_send(canary::net::buffer(&f, sizeof(f)), canary::net::use_awaitable);
        }
      },
      canary::net::detached);

  ioc.run();
}