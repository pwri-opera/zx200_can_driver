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
        sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::send_handle, this));
        send_timer.expires_at(send_timer.expiry() + boost::asio::chrono::milliseconds(cmd_interval));
        send_timer.async_wait(boost::bind(&can_handler::send_pi_cmd1, this));
    }

    void send_pi_cmd2()
    {
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
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::send_handle, this));
      send_timer1.expires_at(send_timer1.expiry() + boost::asio::chrono::milliseconds(cmd1_interval));
      send_timer1.async_wait(boost::bind(&can_handler::send_pi_cmd2, this));
    }

    void send_machine_setting_cmd()
    {
      frame f = {};
      f.header.id(0x18FF660A);
      f.header.extended_format(true);
      f.header.payload_length(8);
      f.payload[0] = setting_cmd->engine_rpm;
      f.payload[1] = std::uint8_t(
          setting_cmd->yellow_lamp_cmd << 4 | setting_cmd->status_notice << 3 |setting_cmd->rabbit_turtle_mode << 2 | setting_cmd->power_eco_mode);
      f.payload[7] = alive_cnt++;

      // sock.send(canary::net::buffer(&f, sizeof(f)));
      sock.async_send(canary::net::buffer(&f, sizeof(f)), boost::bind(&can_handler::send_handle, this));
      send_timer2.expires_at(send_timer2.expiry() + boost::asio::chrono::milliseconds(cmd2_interval));
      send_timer2.async_wait(boost::bind(&can_handler::send_machine_setting_cmd, this));
    }

    void send_handle() {}

    void start_receive()
    {
      sock.async_receive(canary::net::buffer(&recv_f, sizeof(recv_f)), boost::bind(&can_handler::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
      // wprintw(gwUI, "Received CAN frame, id: %x, len: %ld\n", recv_f.header.id(), recv_f.header.payload_length());
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
    boost::shared_ptr<pilot_pressure_cmd1> pi_cmd1;
    boost::shared_ptr<pilot_pressure_cmd2> pi_cmd2;
    boost::shared_ptr<machine_setting_cmd> setting_cmd;
    std::uint8_t alive_cnt;
    frame recv_f;
};

bool processKey(int com, pilot_pressure_cmd1 &cmd1, pilot_pressure_cmd2 &cmd2, machine_setting_cmd &cmd3);

int main(int argc, char **argv)
{
  //ncurses setting
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

  canary::net::io_context ioc;
  boost::thread t(boost::bind(&boost::asio::io_context::run, &ioc));

  pilot_pressure_cmd1 cmd1 = {};
  pilot_pressure_cmd2 cmd2 = {};
  machine_setting_cmd cmd3 = {};

  while (true)
  {
    com = wgetch(gwUI);
    if (com != ERR)
      doit = processKey(com, cmd1,cmd2,cmd3);

    if (doit)
    {
      handler.set_pilot_pressure_cmd1(cmd1);
      handler.set_pilot_pressure_cmd2(cmd2);
      handler.set_machine_setting_cmd(cmd3);
    }
    else
    {
      //停止信号を送信してから終了するようにしたい
      break;
    }

    werase(gwSub[1]);
    wprintw(gwSub[1], "boom raise   %d [MPa]\n", cmd1.boom_raise);
    wprintw(gwSub[1], "boom lower   %d [MPa]\n", cmd1.boom_lower);
    wprintw(gwSub[1], "arm dump     %d [MPa]\n", cmd1.arm_dump);
    wprintw(gwSub[1], "arm crowd    %d [MPa]\n", cmd1.arm_crowd);
    wprintw(gwSub[1], "bucket dump  %d [MPa]\n", cmd1.bucket_dump);
    wprintw(gwSub[1], "bucket crowd %d [MPa]\n", cmd1.bucket_crowd);
    wprintw(gwSub[1], "swing right  %d [MPa]\n", cmd1.swing_right);
    wprintw(gwSub[1], "swing left   %d [MPa]\n\n", cmd1.swing_left);

    wprintw(gwSub[1], "Engine RPM   %d [rpm]\n", cmd3.engine_rpm*10);

    wrefresh(gwSub[1]);

    wrefresh(gwUI);
  }
  delwin(gwSub[0]);
  delwin(gwSub[1]);
  delwin(gwSub[2]);
  delwin(gwUI);
  endwin();

  std::cout << "Bye,bye...." << std::endl;

  return 0;
}

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

bool processKey(int com, pilot_pressure_cmd1 &cmd1, pilot_pressure_cmd2 &cmd2, machine_setting_cmd &cmd3)
{
  static int boom_cmd(0), arm_cmd(0), bucket_cmd(0), swing_cmd(0), engine_rpm(50);
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
  case '+':
    engine_rpm += 10;
    break;
  case '-':
    engine_rpm -= 10;
    break;
  case ' ':
    boom_cmd = arm_cmd = bucket_cmd = swing_cmd = 0;
    engine_rpm = 0;
    break;

  case 'q': //end
    boom_cmd = arm_cmd = bucket_cmd = swing_cmd = 0;
    engine_rpm = 0;
    return false;
  }

  if (boom_cmd >= 0)
  {
    cmd1.boom_raise = lever_clamp(boom_cmd);
    cmd1.boom_lower = 0;
  }
  else
  {
    cmd1.boom_raise = 0;
    cmd1.boom_lower = abs(lever_clamp(boom_cmd));
  }
  if (arm_cmd >= 0)
  {
    cmd1.arm_dump = lever_clamp(arm_cmd);
    cmd1.arm_crowd = 0;
  }
  else
  {
    cmd1.arm_dump = 0;
    cmd1.arm_crowd = abs(lever_clamp(arm_cmd));
  }
  if (bucket_cmd >= 0)
  {
    cmd1.bucket_dump = lever_clamp(bucket_cmd);
    cmd1.bucket_crowd = 0;
  }
  else
  {
    cmd1.bucket_dump = 0;
    cmd1.bucket_crowd = abs(lever_clamp(bucket_cmd));
  }
  if (swing_cmd >= 0)
  {
    cmd1.swing_right = lever_clamp(swing_cmd);
    cmd1.swing_left = 0;
  }
  else
  {
    cmd1.swing_right = 0;
    cmd1.swing_left = abs(lever_clamp(swing_cmd));
  }
  if(engine_rpm >= 0){
    cmd3.engine_rpm = lever_clamp(engine_rpm);
  }else{
    cmd3.engine_rpm = 0;
    engine_rpm = 0;
  }
  return true;
}
