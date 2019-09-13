#pragma once
//------------------------------------------------------------------------------
//
//   Copyright 2018-2019 Fetch.AI Limited
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//------------------------------------------------------------------------------

#include <memory>
#include <queue>
#include <vector>

#include "core/mutex.hpp"
#include "network/uri.hpp"
#include "network/management/network_manager.hpp"
#include "network/muddle/muddle.hpp" 
#include "network/service/service_client.hpp"

#include "dmlf/ilearner_networker.hpp"
#include "dmlf/iupdate.hpp"
//#include "dmlf/dmlf_rpc.hpp"
#include "dmlf/dmlf_service.hpp"


namespace fetch {
namespace dmlf {

uint16_t ephem_port_();

class MuddleLearnerNetworker: public ILearnerNetworker
{
public:
  using IUpdatePtr   = std::shared_ptr<IUpdate>;
  using QueueUpdates = std::priority_queue<IUpdatePtr, std::vector<IUpdatePtr>, std::greater<IUpdatePtr>>;
  using Uri          = fetch::network::Uri;
  
  MuddleLearnerNetworker(std::vector<Uri> peers, std::shared_ptr<network::NetworkManager> nnmm = nullptr)
  : nm_{nnmm}, nm_mine_{false}
  , initial_peers_{peers}
  , listen_port_{ephem_port_()}
  {
    if(!nm_)
    {
      nm_ = std::make_shared<network::NetworkManager>("dmlf", 4);
      nm_mine_ = true;
    }
    
    //muddle_ = muddle::Muddle::CreateMuddle(muddle::NetworkId{"DMLF"}, *nm_);
    updates_ostream_ = std::make_shared<DmlfUpdateService>(*nm_, listen_port_);
    std::cout << "Listening on port " << listen_port_ << std::endl;

    for (auto& uri : initial_peers_)
    {
      fetch::network::TCPClient connection(*nm_);
      connection.Connect(uri.AsPeer().address(), uri.AsPeer().port());
      auto client = std::make_shared<DmlfUpdateClient>(connection, *nm_);
      updates_istreams_.push_back(client);
      std::cout << "Added connection to " << uri.AsPeer().address() << ":" << uri.AsPeer().port() << std::endl;
    }
  }
  
  virtual ~MuddleLearnerNetworker()
  {
  }
  
  void start();

  virtual void pushUpdate( std::shared_ptr<IUpdate> update) override;
  virtual std::size_t getUpdateCount() const override;
  virtual std::shared_ptr<IUpdate> getUpdate() override;
  virtual std::size_t getCount() override;

protected:
private:
  using Mutex  = fetch::Mutex;
  using Lock   = std::unique_lock<Mutex>;
  //using Muddle = fetch::muddle::Muddle;
  //using Client = fetch::muddle::rpc::Client; 
  using DmlfUpdateService    = fetch::subscribe::DmlfUpdateService;
  using DmlfUpdateClient     = fetch::service::ServiceClient;
  using DmlfUpdateClientPtr  = std::shared_ptr<DmlfUpdateClient>;

  QueueUpdates  updates_;
  mutable Mutex updates_m_; 
  
  std::shared_ptr<network::NetworkManager> nm_;
  bool nm_mine_;
  std::vector<Uri> initial_peers_;
  //std::shared_ptr<Muddle> muddle_;
  uint16_t listen_port_;
  std::shared_ptr<DmlfUpdateService> updates_ostream_;
  std::vector<DmlfUpdateClientPtr> updates_istreams_;

  MuddleLearnerNetworker(const MuddleLearnerNetworker &other) = delete;
  MuddleLearnerNetworker &operator=(const MuddleLearnerNetworker &other) = delete;
  bool operator==(const MuddleLearnerNetworker &other) = delete;
  bool operator<(const MuddleLearnerNetworker &other) = delete;
};

}  // dmlf
}  // fetch
