#pragma once
#include <iostream>

#include <dbcppp/Network.h>
#include <dbcppp/Network2Functions.h>
#include <fstream>

#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>

#include "can_message.hpp"

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
          prop.set_value(*can_bus, sig.RawToPhys(sig.Decode(&f.payload[0])));
        }
      }
    }
  }

  void encode(const std::uint64_t id, auto cmd, frame &f)
  {
    auto iter = messages.find(id);
    if (iter != messages.end())
    {
      const dbcppp::IMessage *msg = iter->second;

      f.header.id(msg->Id());
      f.header.extended_format(true);
      f.header.payload_length(msg->MessageSize());

      for (const dbcppp::ISignal &sig : msg->Signals())
      {
        rttr::property prop = rttr::type::get(cmd).get_property(sig.Name());
        sig.Encode(sig.PhysToRaw((prop.get_value(cmd)).to_double()), &f.payload[0]);
      }
    }
  }

  void burst_print_dbc()
  {
  }

private:
  std::unordered_map<uint64_t, const dbcppp::IMessage *> messages;
  std::unique_ptr<dbcppp::INetwork> net;
  std::shared_ptr<Can_Bus> can_bus;
};