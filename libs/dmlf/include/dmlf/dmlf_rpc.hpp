#pragma once

#include "core/byte_array/byte_array.hpp"
#include "network/service/protocol.hpp"

class UpdateService
{
public:
  static constexpr char const *LOGGING_NAME = "UpdateService";
  
  //void Push(fetch::byte_array::ByteArray upd_bytes) 
  void Push(uint64_t upd_bytes) 
  {
    FETCH_LOG_INFO(LOGGING_NAME, "Received an update ", upd_bytes);
  }
};

class UpdateProtocol : public fetch::service::Protocol
{
public:
  UpdateProtocol(UpdateService &service)
  {
    Expose(1, &service, &UpdateService::Push);
  }
};


