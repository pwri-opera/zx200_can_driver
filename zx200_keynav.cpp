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
    setting_cmd = boost::shared_ptr<machine_setting_cmd>(new machine_setting_cmd { 900,0,0,0,0});
    boom_pi_cmd = arm_pi_cmd = bucket_pi_cmd = swing_pi_cmd = 0;
    engine_rpm = 900;

    // for (const dbcppp::ISignal &sig : pi_cmd1_msg->Signals())
    // {

    // }
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
    wprintw(gwSub[0], "Instructions:\n");
    wprintw(gwSub[0], " Manipulators:\n");
    wprintw(gwSub[0], "  w:boom raise, s:boom lower\n");
    wprintw(gwSub[0], "  e:arm crowd, d:arm dump\n");
    wprintw(gwSub[0], "  r:bucket crowd, f:bucket dump\n");
    wprintw(gwSub[0], "  t:swing right, g:swing left\n\n");
    wprintw(gwSub[0], " Tracks:\n");
    wprintw(gwSub[0], "  u:left front, i:right front\n");
    wprintw(gwSub[0], "  j:left back, k:right back\n\n");
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
      boom_pi_cmd += 0.02*5;
      pilot_pressure_clamp(boom_pi_cmd);
      break;
    case 's':
      boom_pi_cmd -= 0.02*5;
      pilot_pressure_clamp(boom_pi_cmd);
      break;
    case 'e':
      arm_pi_cmd += 0.02*5;
      pilot_pressure_clamp(arm_pi_cmd);
      break;
    case 'd':
      arm_pi_cmd -= 0.02*5;
      pilot_pressure_clamp(arm_pi_cmd);
      break;
    case 'r':
      bucket_pi_cmd += 0.02*5;
      pilot_pressure_clamp(bucket_pi_cmd);
      break;
    case 'f':
      bucket_pi_cmd -= 0.02*5;
      pilot_pressure_clamp(bucket_pi_cmd);
      break;
    case 't':
      swing_pi_cmd += 0.02*5;
      pilot_pressure_clamp(swing_pi_cmd);
      break;
    case 'g':
      swing_pi_cmd -= 0.02*5;
      pilot_pressure_clamp(swing_pi_cmd);
      break;
    case '+':
      engine_rpm += 100;
      break;
    case '-':
      engine_rpm -= 100;
      break;
    case ' ':
      arm_pi_cmd = bucket_pi_cmd = swing_pi_cmd = 0;
      boom_pi_cmd = 0;
      engine_rpm = 900;
      break;

    case 'q': // end
      arm_pi_cmd = bucket_pi_cmd = swing_pi_cmd = 0;
      boom_pi_cmd = 0;
      engine_rpm = 900;
      return false;
    }
    if (boom_pi_cmd >= 0)
    {
      pi_cmd1->boom_up = boom_pi_cmd; // boom_cmd);
      pi_cmd1->boom_down = 0;
    }
    else
    {
      pi_cmd1->boom_up = 0;
      pi_cmd1->boom_down = abs(boom_pi_cmd);// abs(boom_cmd));
    }
    if (arm_pi_cmd >= 0)
    {
      pi_cmd1->arm_crowd = arm_pi_cmd;
      pi_cmd1->arm_dump = 0;
    }
    else
    {
      pi_cmd1->arm_crowd = 0;
      pi_cmd1->arm_dump = abs(arm_pi_cmd);
    }
    if (bucket_pi_cmd >= 0)
    {
      pi_cmd1->bucket_crowd = bucket_pi_cmd;
      pi_cmd1->bucket_dump = 0;
    }
    else
    {
      pi_cmd1->bucket_crowd = 0;
      pi_cmd1->bucket_dump = abs(bucket_pi_cmd);
    }
    if (swing_pi_cmd >= 0)
    {
      pi_cmd1->swing_right = swing_pi_cmd;
      pi_cmd1->swing_left = 0;
    }
    else
    {
      pi_cmd1->swing_right = 0;
      pi_cmd1->swing_left = abs(swing_pi_cmd);
    }
    if (engine_rpm >= 0)
    {
      setting_cmd->engine_rpm = engine_rpm; // engine_rpm);
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

    wprintw(gwSub[1], " boom up   %2.2f [MPa]\n", pi_cmd1->boom_up);
    wprintw(gwSub[1], " boom down   %2.2f [MPa]\n", pi_cmd1->boom_down);
    wprintw(gwSub[1], " arm crowd     %2.2f [MPa]\n", pi_cmd1->arm_crowd);
    wprintw(gwSub[1], " arm dump    %2.2f [MPa]\n", pi_cmd1->arm_dump);
    wprintw(gwSub[1], " bucket crowd  %2.2f [MPa]\n", pi_cmd1->bucket_crowd);
    wprintw(gwSub[1], " bucket dump %2.2f [MPa]\n", pi_cmd1->bucket_dump);
    wprintw(gwSub[1], " swing right  %2.2f [MPa]\n", pi_cmd1->swing_right);
    wprintw(gwSub[1], " swing left   %2.2f [MPa]\n\n", pi_cmd1->swing_left);
    wprintw(gwSub[1], " engine rpm   %d [rpm]\n", setting_cmd->engine_rpm);

    werase(gwSub[2]);
    wprintw(gwSub[2], "State:\n");
    wprintw(gwSub[2], "  engine rpm   %d [rpm]\n", 0);

    wrefresh(gwSub[1]);
    wrefresh(gwSub[2]);

    wrefresh(gwUI);
  }
  WINDOW *gwSub[3], *gwUI;

private:
  void pilot_pressure_clamp(double &input)
  {
    if (input > 0.02*250)
    {
      input = 0.02 * 250;
    }
    else if (input < -0.02 * 250)
    {
      input = -0.02 * 250;
    }
  }

  boost::shared_ptr<pilot_pressure_cmd1> pi_cmd1;
  boost::shared_ptr<pilot_pressure_cmd2> pi_cmd2;
  boost::shared_ptr<machine_setting_cmd> setting_cmd;
  double boom_pi_cmd, arm_pi_cmd, bucket_pi_cmd, swing_pi_cmd;
  int engine_rpm;
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

    // usleep(10000);
  }

  std::cout << "Bye,bye...." << std::endl;

  return 0;
}