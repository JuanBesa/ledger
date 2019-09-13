#pragma once

#include "core/byte_array/byte_array.hpp"
#include "network/service/server.hpp"
#include "network/tcp/tcp_server.hpp"
#include "protocols/fetch_protocols.hpp"
#include "protocols/subscribe/protocol.hpp"

#include <memory>

namespace fetch {
namespace subscribe {

  

class DmlfUpdateService : public service::ServiceServer<fetch::network::TCPServer>
{
public:
  static constexpr char const *LOGGING_NAME = "DmlfUpdateService";

  DmlfUpdateService(fetch::network::NetworkManager &nm, uint16_t tcp_port)
    : ServiceServer(tcp_port, nm)
  {
    subscribeProto_ = std::make_unique<protocols::SubscribeProtocol>();
    this->Add(protocols::FetchProtocols::SUBSCRIBE_PROTO, subscribeProto_.get());
  }
  
  void SendUpdate(byte_array::ByteArray& update)
  {
    subscribeProto_->SendMessage(update);
  }

private:
  std::unique_ptr<protocols::SubscribeProtocol> subscribeProto_;
};

}  // namespace subscribe
}  // namespace fetch


