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

#include <boost/bind.hpp>

#include <ncurses.h>

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

int processKey(int com, Lever_cmd &lever_cmd);

void lever_cmd_send(boost::asio::deadline_timer *t, canary::raw::socket *sock, Lever_cmd *cmd)
{
  frame f = {};
  f.header.id(0x12345678);
  f.header.extended_format(true);
  f.header.payload_length(8);
  f.payload[0] = cmd->boom_raise;
  f.payload[1] = cmd->boom_lower;
  f.payload[2] = cmd->arm_dump;
  f.payload[3] = cmd->arm_crowd;
  f.payload[4] = cmd->bucket_dump;
  f.payload[5] = cmd->bucket_crowd;
  f.payload[6] = cmd->swing_left;
  f.payload[7] = cmd->swing_right;

  sock->send(canary::net::buffer(&f, sizeof(f)));

  t->expires_at(t->expires_at() + boost::posix_time::millisec(10));
  t->async_wait(boost::bind(lever_cmd_send, t, sock, cmd));
}

int main(int argc, char **argv)
{
  // ncurses window setting
  WINDOW *gwSub[3], *gwUI;

  int iport;
  int i;
  int status;
  char c;
  char line[128];
  char com;
  int rows, cols;
  int ver, serial;
  int logOdo = 0;
  bool doit = true;

  initscr();
  cbreak();
  noecho();
  getmaxyx(stdscr, rows, cols);

  gwSub[0] = newwin(rows / 2 - 1, cols / 3 - 1, 0, cols / 3 * 0);
  gwSub[1] = newwin(rows / 2 - 1, cols / 3 - 1, 0, cols / 3 * 1);
  gwSub[2] = newwin(rows / 2 - 1, cols / 3 - 1, 0, cols / 3 * 2);
  scrollok(gwSub[0], TRUE);
  scrollok(gwSub[1], TRUE);
  scrollok(gwSub[2], TRUE);
  mvvline(0, cols / 3 - 1, '|', rows / 2);
  mvvline(0, cols / 3 * 2 - 1, '|', rows / 2);
  gwUI = newwin(rows / 2, cols, rows / 2 + 1, 0);
  scrollok(gwUI, TRUE);
  wtimeout(gwUI, 100);

  mvhline(rows / 2, 0, '-', cols);
  refresh();

  werase(gwSub[0]);
  wprintw(gwSub[0], "simple keybord controler\n\n");
  wprintw(gwSub[0], " Instructions\n");
  wprintw(gwSub[0], "  w:boom raise, s:boom lower\n");
  wprintw(gwSub[0], "  e:arm dump, d:arm crowd\n");
  wprintw(gwSub[0], "  r:bucket dump, f:bucket crowd\n");
  wprintw(gwSub[0], "  t:swing right, g:swing left\n\n");

  wprintw(gwSub[0], "  [SPACE]   All stop\n");
  wprintw(gwSub[0], "   q        Quit ");

  wrefresh(gwSub[0]);

  //io setting
  canary::net::io_context ioc;

  const auto idx = canary::get_interface_index("vcan0");
  auto const ep = canary::raw::endpoint{idx};
  canary::raw::socket sock{ioc, ep};


  frame f = {};
  Lever_cmd lever_cmd = {};

  canary::net::co_spawn(
      ioc,
      [&ioc, &sock, &f, &gwUI]() -> canary::net::awaitable<void>
      {
        while (true)
        {
          co_await sock.async_receive(canary::net::buffer(&f, sizeof(f)), canary::net::use_awaitable);
          wprintw(gwUI, "Received CAN frame, id: %x, len: %ld\n", f.header.id(), f.header.payload_length());
        }
      },
      canary::net::detached);

  boost::asio::deadline_timer timer(ioc, boost::posix_time::millisec(1000));
  timer.async_wait(boost::bind(lever_cmd_send, &timer, &sock, &lever_cmd));

  boost::thread t(boost::bind(&boost::asio::io_context::run, &ioc));

  while(doit){
    com = wgetch(gwUI);
    if (com != ERR)
      doit = processKey(com,lever_cmd);

    if (!doit){
      //停止信号を送信してから終了するようにしたい
      break;
    }

    werase(gwSub[1]);
    wprintw(gwSub[1], "boom raise   %d [MPa]\n", lever_cmd.boom_raise);
    wprintw(gwSub[1], "boom lower   %d [MPa]\n", lever_cmd.boom_lower);
    wprintw(gwSub[1], "arm dump     %d [MPa]\n", lever_cmd.arm_dump);
    wprintw(gwSub[1], "arm crowd    %d [MPa]\n", lever_cmd.arm_crowd);
    wprintw(gwSub[1], "bucket dump  %d [MPa]\n", lever_cmd.bucket_dump);
    wprintw(gwSub[1], "bucket crowd %d [MPa]\n", lever_cmd.bucket_crowd);
    wprintw(gwSub[1], "swing right  %d [MPa]\n", lever_cmd.swing_right);
    wprintw(gwSub[1], "swing left   %d [MPa]\n", lever_cmd.swing_left);

    wrefresh(gwSub[1]);


    wrefresh(gwUI);
  }

  delwin(gwSub[0]);
  delwin(gwSub[1]);
  delwin(gwSub[2]);
  delwin(gwUI);
  endwin();
  std::cout << "Bye,bye...." << std::endl;
}

int lever_clamp(int &input){
  if (input > 250)
  {
    input = 250;
  }else if(input < -250)
  {
    input = -250;
  }
  return input;
}

int processKey(int com, Lever_cmd &lever_cmd)
{
  static int boom_cmd(0), arm_cmd(0), bucket_cmd(0), swing_cmd(0);
  struct timeval ts;
  gettimeofday(&ts, NULL);
  switch (com)
  {
  case 'w':
    boom_cmd += 5;
    break;
  case 's':
    boom_cmd -= 5;
    break;
  case 'e':
    arm_cmd += 5;
    break;
  case 'd':
    arm_cmd -= 5;
    break;
  case 'r':
    bucket_cmd += 5;
    break;
  case 'f':
    bucket_cmd -= 5;
    break;
  case 't':
    swing_cmd += 5;
    break;
  case 'g':
    swing_cmd -= 5;
    break;

  case ' ':
    boom_cmd = arm_cmd = bucket_cmd = swing_cmd = 0;
    break;

  case 'q'://end
    boom_cmd = arm_cmd = bucket_cmd = swing_cmd = 0;
    return false;
  }

  if(boom_cmd >= 0){
    lever_cmd.boom_raise = lever_clamp(boom_cmd);
    lever_cmd.boom_lower = 0;
  }else{
    lever_cmd.boom_raise = 0;
    lever_cmd.boom_lower = abs(lever_clamp(boom_cmd));
  }
  if (arm_cmd >= 0)
  {
    lever_cmd.arm_dump = lever_clamp(arm_cmd);
    lever_cmd.arm_crowd = 0;
  }
  else
  {
    lever_cmd.arm_dump = 0;
    lever_cmd.arm_crowd = abs(lever_clamp(arm_cmd));
  }
  if (bucket_cmd >= 0)
  {
    lever_cmd.bucket_dump = lever_clamp(bucket_cmd);
    lever_cmd.bucket_crowd = 0;
  }
  else
  {
    lever_cmd.bucket_dump = 0;
    lever_cmd.bucket_crowd = abs(lever_clamp(bucket_cmd));
  }
  if (swing_cmd >= 0)
  {
    lever_cmd.swing_right = lever_clamp(swing_cmd);
    lever_cmd.swing_left = 0;
  }
  else
  {
    lever_cmd.swing_right = 0;
    lever_cmd.swing_left = abs(lever_clamp(swing_cmd));
  }

  return true;
}
