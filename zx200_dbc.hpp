#pragma once
#include <iostream>

#include <dbcppp/Network.h>
#include <dbcppp/Network2Functions.h>
#include <fstream>

#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>

#include "zx200_dbc_struct.hpp"

struct frame
{
  canary::frame_header header;
  std::array<std::uint8_t, 8> payload;
};

class zx200_dbc
{
public:
  zx200_dbc(std::string dbc_path)
  {
    std::ifstream idbc(dbc_path);
    net = dbcppp::INetwork::LoadDBCFromIs(idbc);

    for (const dbcppp::IMessage &msg : net->Messages())
    {
      messages.insert(std::make_pair(msg.Id(), &msg));
    }
    can_bus = std::make_shared<Can_Bus>();
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
          std::string name = msg->Name() + "::" + sig.Name();
          rttr::property prop = rttr::type::get(*can_bus).get_property(name);
          if(!prop)
            std::cout << "error: " << name << std::endl;
          bool ret = false;
          ret = prop.set_value(*can_bus, (double)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (bool)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (std::uint8_t)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (std::int8_t)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (std::uint16_t)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (std::int16_t)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (std::uint32_t)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (std::int32_t)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (std::uint64_t)sig.RawToPhys(sig.Decode(&f.payload[0])));
          if (!ret)
            ret = prop.set_value(*can_bus, (std::int64_t)sig.RawToPhys(sig.Decode(&f.payload[0])));
        }
      }
    }
  }

  void encode(auto cmd, frame &f)
  {
    auto iter = messages.find(cmd.Id());
    if (iter != messages.end())
    {
      const dbcppp::IMessage *msg = iter->second;

      f.header.id(msg->Id());
      f.header.extended_format(true);
      f.header.payload_length(msg->MessageSize());

      for (const dbcppp::ISignal &sig : msg->Signals())
      {
        std::string name = msg->Name() + "::" + sig.Name();
        rttr::property prop = rttr::type::get(cmd).get_property(name);
        sig.Encode(sig.PhysToRaw((prop.get_value(cmd)).to_double()), &f.payload[0]);
      }
    }
  }

  void burst_print_dbc()
  {
  }

  void get_can_bus_msg(auto &decoded_can_msg_struct)
  {
    decoded_can_msg_struct = *can_bus;
  }
private:
  std::unordered_map<uint64_t, const dbcppp::IMessage *> messages;
  std::unique_ptr<dbcppp::INetwork> net;
  std::shared_ptr<Can_Bus> can_bus;
};