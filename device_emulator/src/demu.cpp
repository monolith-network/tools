#include "config.hpp"
#include "environment.hpp"
#include "interfaces/sensor_if.hpp"

#include <chrono>
#include <thread>
#include <csignal>
#include <iostream>
#include <nettle/Writer.hpp>
#include <crate/registrar/submitter.hpp>
#include <crate/networking/message_writer.hpp>

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
   crate::registrar::node_v1 demu(g_config.device_name);

   // Add sensors to registrar object
   //
   for(auto sensor : g_config.sensors) {
      crate::registrar::node_v1::sensor v1_sensor;
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
   crate::registrar::submitter registrar_submitter(g_config.registrar_address, g_config.registrar_port);
   if (registrar_submitter.submit(demu) != crate::registrar::submitter::result::SUCCESS) {
      std::cerr << "\nFailed to register node with registrar" << std::endl;
      std::exit(1);
   }

   auto writer = crate::networking::message_writer(
      g_config.submission_server_address, 
      g_config.submission_server_port
   );

   std::cout << "SUCCESS\n";

   std::cout << "Metric Destination::" << g_config.submission_server_address << ":"
               << g_config.submission_server_port << std::endl;

   while (true) {
      std::this_thread::sleep_for(1s);

      for(auto sensor : g_config.sensors) {
         auto reading = sensor->get_value();

         std::string encoded_reading;
         reading.encode_to(encoded_reading);

         bool okay {false};
         writer.write(encoded_reading, okay);
         if (!okay) {
            std::cerr << "Writer has experienced an error\n";
            std::exit(1);
         }
      }
   }

   return 0;
}