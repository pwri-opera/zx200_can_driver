#pragma once

#include <dbcppp/Network.h>
#include <dbcppp/Network2Functions.h>
#include <fstream>

#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>


// 新しく追加
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/sequence.hpp>

// 

#define pi_cmd1_interval 10
#define pi_cmd2_interval 10
#define setting_cmd_interval 50

//自動生成したい
struct pilot_pressure_cmd1
{
  double boom_up;
  double boom_down;
  double arm_crowd;
  double arm_dump;
  double bucket_crowd;
  double bucket_dump;
  double swing_right;
  double swing_left;
};

BOOST_FUSION_ADAPT_STRUCT(
  pilot_pressure_cmd1,
  boom_up,
  boom_down,
  arm_crowd,
  arm_dump,
  bucket_crowd,
  bucket_dump,
  swing_right,
  swing_left,
)

struct SG_pi_cmd1
{
  double data;
  double factor;
};

struct BO_pi_cmd1
{
  const uint32_t id = 0x18FF640A;
  SG_pi_cmd1 boom_up;
  SG_pi_cmd1 boom_down;
  uint64_t cycle_time;
};

//自動生成したい
struct pilot_pressure_cmd2
{
  double track_right_forward;
  double track_left_forward;
  double track_right_backward;
  double track_left_backward;
  double attachment_a;
  double attachment_b;
  double assist_a;
  double assist_b;
};

struct machine_setting_cmd
{
  int engine_rpm;
  std::uint8_t power_eco_mode;
  std::uint8_t travel_speed_mode;
  std::uint8_t working_mode_notice;
  std::uint8_t yellow_led_mode;
  // byte3-7 is not assigned
  std::uint8_t alive_counter;
};


struct front_angle
{
  double boom_angle;
  double arm_angle;
  double bucket_angle;
  double swing_angle;
};

struct frame
{
  canary::frame_header header;
  std::array<std::uint8_t, 8> payload;
};


class zx200_dbc
{
public:
  zx200_dbc()
  {
    std::ifstream idbc("/home/genki/zx200.dbc");
    net = dbcppp::INetwork::LoadDBCFromIs(idbc);
    BO_pi_cmd1 bo_pi;

    for (const dbcppp::IMessage &msg : net->Messages())
    {
      messages.insert(std::make_pair(msg.Id(), &msg));
      if (msg.Id() == 0x18FF640A)
      {
        bo_pi.boom_up.factor = msg.Signals_Get(0).Factor();
        bo_pi.boom_down.factor = msg.Signals_Get(0).Factor();
      }
    }
    
    auto iter = messages.find(0x18FF640A | 0x80000000);
    pi_cmd1_msg = iter->second;

  }

  void decode(frame f){
    auto iter = messages.find(f.header.id() | 0x80000000);
    if (iter != messages.end())
    {
      const dbcppp::IMessage *msg = iter->second;
      for (const dbcppp::ISignal &sig : msg->Signals())
      {
        const dbcppp::ISignal *mux_sig = msg->MuxSignal();
        if (sig.MultiplexerIndicator() != dbcppp::ISignal::EMultiplexer::MuxValue ||
            (mux_sig && mux_sig->Decode(&f.payload) == sig.MultiplexerSwitchValue()))
        {
          if (msg->Name() == "Front_Angle")
          {
            msg->Signals_Get(0).Decode(&f.payload);
          }
          else if (msg->Name() == "Front_Angular_Velocity")
          {

          }
            // std::cout << "\t\t" << sig.Name() << "=" << sig.RawToPhys(sig.Decode(frame.data)) << sig.Unit() << "\n";
        }
      }

      if(msg->Name() == "Front_Angle")
      {
        // f_angle = msg->Signals_Get(0).Encode


      }
      else if (msg->Name() == "xxx")
      {

      }
    }
  }

  // void pi_cmd1_encode(std::map<std::string, double> pi_cmd1, frame &f)
  void pi_cmd1_encode(pilot_pressure_cmd1 cmd, frame &f)
  {
    f.header.id(pi_cmd1_msg->Id());
    f.header.extended_format(pi_cmd1_msg->Id() && 0x8000000);
    f.header.payload_length(pi_cmd1_msg->MessageSize());

    int i = 0;
    for (const dbcppp::ISignal &sig : pi_cmd1_msg->Signals())
    {
      // sig.Encode(sig.PhysToRaw(boost::fusion::at_c<0>(cmd), &f.payload[0]);
    }


  }
  void front_angle_decode(frame f)
  {
    auto iter = messages.find(f.header.id() | 0x80000000);
    if (iter != messages.end())
    {
      const dbcppp::IMessage *msg = iter->second;
      for (const dbcppp::ISignal &sig : msg->Signals())
      {
        // sig.Encode(sig.PhysToRaw(pi_cmd1[sig.Name()]), &f.payload);
        sig.Decode(&f.payload);
      }
    }
  }


  void create_dbc_map(std::map<std::string, double> &pi_cmd1x)
  {
    for (const dbcppp::ISignal &sig : pi_cmd1_msg->Signals())
    {
      pi_cmd1x.insert(std::pair(sig.Name(), 0));
    }
  }

private:
  void map_db_from_dbc()
  {

  }
  std::unordered_map<uint64_t, const dbcppp::IMessage *> messages;
  std::unique_ptr<dbcppp::INetwork> net;
  const dbcppp::IMessage *pi_cmd1_msg;
  front_angle f_angle;

  std::map<std::string, std::map<std::string, std::variant<std::string, int, double>>> cmdxx;
};