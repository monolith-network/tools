#include "config.hpp"
#include "environment.hpp"
#include "interfaces/sensor_if.hpp"

#include <chrono>
#include <thread>
#include <csignal>
#include <iostream>
#include <nettle/Writer.hpp>
#include <crate/registrar/helper.hpp>
#include <crate/networking/message_writer.hpp>
#include <crate/metrics/helper.hpp>

using namespace std::chrono_literals;

namespace {
   demu::configuration g_config;
}


void signal_ignore_handler(int signum)
{
   std::cout << "Ignoring signal: " 
               << signum 
               << "\n";
}

int main(int argc, char **argv) {

   if (argc != 2) {
      std::cout << "Usage : " << argv[0] << " config.toml" << std::endl;
      return 1; 
   }

   signal(SIGPIPE, signal_ignore_handler);

   demu::environment env;
   auto config = demu::load_config(argv[1], env);
   if (!config.has_value()) {
      std::cerr << "Failed to load config" << std::endl;
      return 1;
   }
   g_config = *config;

   env.configure(g_config.env_details);

   // Build a node object to register with registrar
   //
   crate::registrar::node_v1_c demu(g_config.device_name);

   // Add sensors to registrar object
   //
   for(auto sensor : g_config.sensors) {
      crate::registrar::node_v1_c::sensor v1_sensor;
      v1_sensor.id = sensor->get_uuid();
      v1_sensor.description = "A sensor of mysterious origins";
      v1_sensor.type = demu::sensor_type_to_string(sensor->get_type());

      if (!demu.add_sensor(v1_sensor)) {
         std::cerr << "Failed to add sensor : " << sensor->get_uuid() << std::endl;
         std::exit(1);
      }
   }

   std::cout << "Registering demu instance with registrar... ";

   // Submit the node for registration
   //
   crate::registrar::helper_c registrar_helper(g_config.address, g_config.http_port);
   if (registrar_helper.submit(demu) != crate::registrar::helper_c::result::SUCCESS) {
      std::cerr << "\nFailed to register node with registrar" << std::endl;
      std::exit(1);
   }

   crate::metrics::helper_c metric_helper(
      crate::metrics::helper_c::endpoint_type_e::HTTP,
      g_config.address,
      g_config.http_port
   );

   std::cout << "SUCCESS\n";

   std::cout << "Metric Destination::" << g_config.address << ":"
               << g_config.metric_submission_port << std::endl;

   while (true) {
      std::this_thread::sleep_for(1s);

      for(auto sensor : g_config.sensors) {
         auto reading = sensor->get_value();

         if (metric_helper.submit(reading)  != crate::metrics::helper_c::result::SUCCESS) {
            std::cerr << "Failed to write metric" << std::endl;
            std::exit(1);
         }
      }
   }

   return 0;
}