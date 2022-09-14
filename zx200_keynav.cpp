#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>
// #include <canary/socket_options.hpp>
#include <iostream>
#include <string>

#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <ncurses.h>

#include "zx200_can.hpp"


#include "zx200_dbc.hpp"

#define cmd_interval 10
#define cmd1_interval 10
#define cmd2_interval 50



class zx200_keynav : public zx200_can
{
  // using zx200_can::zx200_can;

public:
  zx200_keynav(boost::asio::io_context &io, std::string can_port) : zx200_can(io, can_port)
  {
    // gwUI = ui;
    pi_cmd1 = boost::shared_ptr<pilot_pressure_cmd1>(new pilot_pressure_cmd1{});
    pi_cmd2 = boost::shared_ptr<pilot_pressure_cmd2>(new pilot_pressure_cmd2{});
    setting_cmd = boost::shared_ptr<machine_setting_cmd>(new machine_setting_cmd { 50,0,0,0,0});
    boom_cmd = arm_cmd = bucket_cmd = swing_cmd = 0;
    engine_rpm = 50;

    int iport;
    int i;
    int status;
    char c;
    char line[128];
    int rows, cols;
    int ver, serial;
    int logOdo = 0;

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
  }
  ~zx200_keynav()
  {
    delwin(gwSub[0]);
    delwin(gwSub[1]);
    delwin(gwSub[2]);
    delwin(gwUI);
    endwin();
  }
  bool update_command(int com)
  {

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
    case '+':
      engine_rpm += 10;
      break;
    case '-':
      engine_rpm -= 10;
      break;
    case ' ':
      boom_cmd = arm_cmd = bucket_cmd = swing_cmd = 0;
      engine_rpm = 50;
      break;

    case 'q': // end
      boom_cmd = arm_cmd = bucket_cmd = swing_cmd = 0;
      engine_rpm = 50;
      return false;
    }
    if (boom_cmd >= 0)
    {
      pi_cmd1->boom_raise = lever_clamp(boom_cmd);
      pi_cmd1->boom_lower = 0;
    }
    else
    {
      pi_cmd1->boom_raise = 0;
      pi_cmd1->boom_lower = abs(lever_clamp(boom_cmd));
    }
    if (arm_cmd >= 0)
    {
      pi_cmd1->arm_dump = lever_clamp(arm_cmd);
      pi_cmd1->arm_crowd = 0;
    }
    else
    {
      pi_cmd1->arm_dump = 0;
      pi_cmd1->arm_crowd = abs(lever_clamp(arm_cmd));
    }
    if (bucket_cmd >= 0)
    {
      pi_cmd1->bucket_dump = lever_clamp(bucket_cmd);
      pi_cmd1->bucket_crowd = 0;
    }
    else
    {
      pi_cmd1->bucket_dump = 0;
      pi_cmd1->bucket_crowd = abs(lever_clamp(bucket_cmd));
    }
    if (swing_cmd >= 0)
    {
      pi_cmd1->swing_right = lever_clamp(swing_cmd);
      pi_cmd1->swing_left = 0;
    }
    else
    {
      pi_cmd1->swing_right = 0;
      pi_cmd1->swing_left = abs(lever_clamp(swing_cmd));
    }
    if (engine_rpm >= 0)
    {
      setting_cmd->engine_rpm = lever_clamp(engine_rpm);
    }
    else
    {
      setting_cmd->engine_rpm = 0;
      engine_rpm = 0;
    }
    set_pilot_pressure_cmd1(*pi_cmd1);
    set_pilot_pressure_cmd2(*pi_cmd2);
    set_machine_setting_cmd(*setting_cmd);

    return true;
  }
  void update_window()
  {
    werase(gwSub[1]);
    wprintw(gwSub[1], "Commands:\n");

    wprintw(gwSub[1], " boom raise   %d [MPa]\n", pi_cmd1->boom_raise);
    wprintw(gwSub[1], " boom lower   %d [MPa]\n", pi_cmd1->boom_lower);
    wprintw(gwSub[1], " arm dump     %d [MPa]\n", pi_cmd1->arm_dump);
    wprintw(gwSub[1], " arm crowd    %d [MPa]\n", pi_cmd1->arm_crowd);
    wprintw(gwSub[1], " bucket dump  %d [MPa]\n", pi_cmd1->bucket_dump);
    wprintw(gwSub[1], " bucket crowd %d [MPa]\n", pi_cmd1->bucket_crowd);
    wprintw(gwSub[1], " swing right  %d [MPa]\n", pi_cmd1->swing_right);
    wprintw(gwSub[1], " swing left   %d [MPa]\n\n", pi_cmd1->swing_left);
    wprintw(gwSub[1], " engine rpm   %d [rpm]\n", setting_cmd->engine_rpm * 10);

    werase(gwSub[2]);
    wprintw(gwSub[2], "State:\n");
    wprintw(gwSub[2], "  engine rpm   %d [rpm]\n", 0);

    wrefresh(gwSub[1]);
    wrefresh(gwSub[2]);

    wrefresh(gwUI);
  }
  WINDOW *gwSub[3], *gwUI;

private:
  int lever_clamp(int &input)
  {
    if (input > 250)
    {
      input = 250;
    }
    else if (input < -250)
    {
      input = -250;
    }
    return input;
  }

  boost::shared_ptr<pilot_pressure_cmd1> pi_cmd1;
  boost::shared_ptr<pilot_pressure_cmd2> pi_cmd2;
  boost::shared_ptr<machine_setting_cmd> setting_cmd;
  int boom_cmd, arm_cmd, bucket_cmd, swing_cmd, engine_rpm;
};

bool processKey(int com, pilot_pressure_cmd1 &cmd1, pilot_pressure_cmd2 &cmd2, machine_setting_cmd &cmd3);

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    std::cerr << "Error\n"
              << "Usage: " << argv[0] << " [can port] [baudrate]" << std::endl;
    return -1;
  }
  std::string can_port(argv[1]);
  int baudrate=std::__cxx11::stoi(argv[2]);
  
  canary::net::io_context ioc;
  zx200_keynav keynav(ioc, can_port);

  boost::thread t(boost::bind(&boost::asio::io_context::run, &ioc));

  bool doit = true;

  while (doit)
  {
    char com = wgetch(keynav.gwUI);
    if (com != ERR)
    {
      doit = keynav.update_command(com);
    }
    keynav.update_window();

    usleep(10000);
  }

  std::cout << "Bye,bye...." << std::endl;

  return 0;
}