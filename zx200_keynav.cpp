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

class zx200_keynav : public zx200_can
{
public:
  zx200_keynav(boost::asio::io_context &io, std::string can_port, std::string dbc_path) : zx200_can(io, can_port, dbc_path)
  {
    // gwUI = ui;
    pi_cmd1 = boost::shared_ptr<Pilot_Pressure_Cmd_1>(new Pilot_Pressure_Cmd_1{});
    pi_cmd2 = boost::shared_ptr<Pilot_Pressure_Cmd_2>(new Pilot_Pressure_Cmd_2{});
    setting_cmd = boost::shared_ptr<Machine_Setting_Cmd>(new Machine_Setting_Cmd {});
    boom_pi_cmd = arm_pi_cmd = bucket_pi_cmd = swing_pi_cmd = 0;
    setting_cmd->engine_rpm = engine_rpm = 900;

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
    case 'x': // eco mode
      setting_cmd->power_eco_mode = 0x0;
      break;
    case 'c': // power mode
      setting_cmd->power_eco_mode = 0x1;
      break;
    case 'v': // rabbit mode
      setting_cmd->travel_speed_mode = 0x0;
      break;
    case 'b': // turtle mode
      setting_cmd->travel_speed_mode = 0x1;
      break;
    case 'n': // switch working_mode_notice to teleoperation
      setting_cmd->working_mode_notice = false;
      break;
    case 'm': // switch working_mode_notice to automation
      setting_cmd->working_mode_notice = true;
      break;

    case ','://switch yellow led to off
      setting_cmd->yellow_led_mode = 0x0;
      break;
    case '.': //switch yellow led to blinking
      setting_cmd->yellow_led_mode = 0x1;
      break;
    case '/': //switch  yellow led to steady light
      setting_cmd->yellow_led_mode = 0x2;
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
    Machine_State state;
    zx200_can::zx200_dbc::get_can_bus_msg(state);

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
    wprintw(gwSub[2], "Machine State:\n");
    wprintw(gwSub[2], "  lock_cmd_state   %d [0:off,1:on]\n", state.lock_cmd_state);
    wprintw(gwSub[2], "  unlock_cmd_state   %d [0:off,1:on]\n", state.unlock_cmd_state);
    wprintw(gwSub[2], "  pi_shutoff_state   %d [0:lock,1:unlock]\n", state.pilot_shutoff_valve_state);
    wprintw(gwSub[2], "  zx200x7_system_error   %d [0:ok,1:err]\n", state.zx200x7_system_error);
    wprintw(gwSub[2], "  can_error_pl   %d [0:ok,1:err]\n", state.can_error_pl);
    wprintw(gwSub[2], "  can_error_body   %d [0:ok,1:err]\n", state.can_error_body);
    wprintw(gwSub[2], "  can_error_ict   %d [0:ok,1:err]\n", state.can_error_ict);
    wprintw(gwSub[2], "  lock_receiver_error   %d [0:ok,1:err]\n", state.lock_receiver_error);
    wprintw(gwSub[2], "  emg_stop_recver_error   %d [0:ok,1:err]\n", state.emergency_stop_receiver_error);
    wprintw(gwSub[2], "  control_state   %d [-]\n", state.control_state);
    wprintw(gwSub[2], "  engine_state   %d [0:off,1:one]\n", state.engine_state);
    wprintw(gwSub[2], "  hydraulic_oil_temp   %d [degC]\n", state.hydraulic_oil_temp);
    wprintw(gwSub[2], "  alive_counter   %d [-]\n", state.alive_counter);


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

  boost::shared_ptr<Pilot_Pressure_Cmd_1> pi_cmd1;
  boost::shared_ptr<Pilot_Pressure_Cmd_2> pi_cmd2;
  boost::shared_ptr<Machine_Setting_Cmd> setting_cmd;
  double boom_pi_cmd, arm_pi_cmd, bucket_pi_cmd, swing_pi_cmd;
  int engine_rpm;
};


int main(int argc, char **argv)
{
  if (argc != 3)
  {
    std::cerr << "Error\n"
              << "Usage: " << argv[0] << " [can_port] [dbc_path]" << std::endl;
    return -1;
  }
  std::string can_port(argv[1]);
  std::string dbc_path(argv[2]);

  canary::net::io_context ioc;
  zx200_keynav keynav(ioc, can_port, dbc_path);

  boost::thread t(boost::bind(&boost::asio::io_context::run, &ioc));

  bool doit = true;

  while (doit)
  {
    char com = wgetch(keynav.gwUI);
    doit = keynav.update_command(com);
    keynav.update_window();
    // usleep(10000);
  }

  std::cout << "Bye,bye...." << std::endl;

  return 0;
}