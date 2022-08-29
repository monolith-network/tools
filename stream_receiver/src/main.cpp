#include <iostream>
#include <string>
#include <optional>
#include <toml++/toml.h>
#include <httplib.h>
#include <csignal>
#include <atomic>
#include <crate/networking/message_server.hpp>
#include <crate/metrics/streams/stream_data_v1.hpp>
#include <crate/externals/simplejson/json.hpp>
#include <crate/metrics/streams/helper.hpp>

using namespace std::chrono_literals;
namespace {
   struct configuration {
      // Local
      std::string stream_server_address;
      uint32_t stream_server_port {0};

      // Remote
      std::string address;
      uint32_t http_port {0};
   };

   std::atomic<bool> active {true};

   class receiver : public crate::networking::message_receiver_if {
   public:
      virtual void receive_message(std::string message) override final {

         std::cout << message << std::endl;

         crate::metrics::streams::stream_data_v1 data;
         if (!data.decode_from(message)) {
            std::cerr << "Unable to decode stream data: " << message << std::endl;

            // Kill the server
            active.store(false);
         }
      }
   };

   receiver message_receiver;
   configuration g_config;
   httplib::Client* http_client {nullptr};
   crate::networking::message_server* server {nullptr};
   std::atomic<bool> handling_signal {false};
}

void load_config(const std::string& file) {
   toml::table tbl;
   try {
      tbl = toml::parse_file(file);
   } catch (const toml::parse_error& err) {
      std::cerr << "Unable to parse file : " 
         << *err.source().path 
         << ". Description: " 
         << err.description() 
         << " (" << err.source().begin << ")\n";
      std::exit(1);
   }

   std::optional<std::string> stream_server_address = tbl["server"]["address"].value<std::string>();
   if (stream_server_address.has_value()) {
      g_config.stream_server_address = *stream_server_address;
   } else {
      std::cout << "Missing config for 'server address'\n";
      std::exit(1); 
   }

   std::optional<uint32_t> stream_server_port = tbl["server"]["port"].value<uint32_t>();
   if (stream_server_port.has_value()) {
      g_config.stream_server_port = *stream_server_port;
   } else {
      std::cout << "Missing config for 'stream server port'\n";
      std::exit(1); 
   }

   std::optional<std::string> address = tbl["monolith"]["address"].value<std::string>();
   if (address.has_value()) {
      g_config.address = *address;
   } else {
      std::cout << "Missing config for 'address'\n";
      std::exit(1); 
   }

   std::optional<uint32_t> http_port = tbl["monolith"]["http_port"].value<uint32_t>();
   if (http_port.has_value()) {
      g_config.http_port = *http_port;
   } else {
      std::cout << "Missing config for 'http port'\n";
      std::exit(1); 
   }
   
   
   // Start the server with the receiver that will dump everything
   //
   server = new crate::networking::message_server(
      g_config.stream_server_address, 
      g_config.stream_server_port, 
      &message_receiver);

   if (!server->start()) {
      std::cerr << "Failed to start server" << std::endl;
      std::exit(1);
   }

   // Register with the server with metra 
   //
   http_client = new httplib::Client(g_config.address, g_config.http_port);
}

void handle_response(crate::metrics::streams::helper::result result) {
   switch (result) {
      case crate::metrics::streams::helper::result::SUCCESS: {
         std::cout << "SUCCESS" << std::endl;
         return;
      }
      case crate::metrics::streams::helper::result::UNABLE_TO_REACH_MONOLITH: {
         std::cerr << "UNABLE_TO_REACH_MONOLITH" << std::endl;
         std::exit(1);
      }
      case crate::metrics::streams::helper::result::FAILED_TO_PARSE_RESPONSE: {
         std::cerr << "FAILED_TO_PARSE_RESPONSE" << std::endl;
         std::exit(1);
      }
      case crate::metrics::streams::helper::result::INTERNAL_SERVER_ERROR: {
         std::cerr << "INTERNAL_SERVER_ERROR" << std::endl;
         std::exit(1);
      }
      case crate::metrics::streams::helper::result::UNKNOWN_ERROR: {
         std::cerr << "UNKNOWN_ERROR" << std::endl;
         std::exit(1);
      }
      case crate::metrics::streams::helper::result::BAD_REQUEST: {
         std::cerr << "BAD_REQUEST" << std::endl;
         std::exit(1);
      }
   }
}

void register_as_stream_receiver() {
   std::cout << "Registering as stream receiver" << std::endl;
   crate::metrics::streams::helper helper(g_config.address, g_config.http_port);
   handle_response(helper.register_as_metric_stream_receiver(
      g_config.stream_server_address,
      g_config.stream_server_port
   ));
}

void deregister_as_stream_receiver() {
   std::cout << "Deregistering as stream receiver" << std::endl;
   crate::metrics::streams::helper helper(g_config.address, g_config.http_port);
   handle_response(helper.deregister_as_metric_stream_receiver(
      g_config.stream_server_address,
      g_config.stream_server_port
   ));
}

void handle_signal(int signal) {

   if (handling_signal.load()) {
      return;
   }

   handling_signal.store(true);
   active.store(false);

   std::cout << "\nExiting.." << std::endl;
}

int main(int argc, char** argv) {

   if (argc != 2) {
      std::cout << "Usage : " << argv[0] << " config.toml" << std::endl;
      return 1; 
   }

   signal(SIGHUP , handle_signal);   /* Hangup the process */ 
   signal(SIGINT , handle_signal);   /* Interrupt the process */ 
   signal(SIGQUIT, handle_signal);   /* Quit the process */ 
   signal(SIGILL , handle_signal);   /* Illegal instruction. */ 
   signal(SIGTRAP, handle_signal);   /* Trace trap. */ 
   signal(SIGABRT, handle_signal);   /* Abort. */

   load_config(argv[1]);

   // register with monolith
   //
   register_as_stream_receiver();

   while(active.load()) {
      std::this_thread::sleep_for(1s);
   }

   // Deregister 
   //
   deregister_as_stream_receiver();

   return 0;
}  