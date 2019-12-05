#include "core/byte_array/const_byte_array.hpp"
#include "http/json_response.hpp"
#include "http/middleware/deny_all.hpp"
#include "http/middleware/token_auth.hpp"
#include "http/middleware/telemetry.hpp"

#include "http/response.hpp"
#include "http/server.hpp"
#include "http/validators.hpp"
//#include "json/document.hpp"

#include "telemetry_http_module.hpp"
#include "telemetry/registry.hpp"
#include "telemetry/counter.hpp"

#include "dmlf/colearn/update_store.hpp"
#include "dmlf/colearn/colearn_update.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

using namespace fetch::http;
using namespace fetch::json;

using namespace fetch::dmlf::colearn;

using fetch::byte_array::ConstByteArray;

struct ExampleModule : HTTPModule
{
  using ConstByteArray = fetch::byte_array::ConstByteArray;

  ExampleModule() /*:HTTPModule("ExampleModule", INTERFACE)*/
  {
    Get("/pages", "Gets the pages",
        [](fetch::http::ViewParameters const & /*params*/,
           fetch::http::HTTPRequest const & /*request*/) {
          return fetch::http::HTTPResponse(ConstByteArray("Hello world"));
        });
  }
};

int main()
{
  std::cout << "Hello world\n";
  fetch::network::NetworkManager tm{"NetMgr", 1};

  HTTPServer    server(tm);
  tm.Start();
  server.Start(8080);

  ExampleModule module;
  server.AddModule(module);

  server.AddMiddleware(fetch::http::middleware::Telemetry());                                               

  server.AddModule(fetch::TelemetryHttpModule());

  UpdateStore store;
  store.PushUpdate("algo", "update", ConstByteArray{"a"}, "test", {});
  
  auto counter = fetch::telemetry::Registry::Instance().CreateCounter("sleeps_total", "how many times the server has slept" , {{"server", "http"}});

  std::cout << "HTTP server on port 8080" << std::endl;
  std::cout << "Ctrl-C to stop" << std::endl;
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    counter->increment();
    store.PushUpdate("algo", "update", ConstByteArray{"a"}, "test", {});
  }


  tm.Stop();

  return EXIT_SUCCESS;
}

