#include "config.hpp"
#include "environment.hpp"
#include "interfaces/sensor_if.hpp"
#include "interfaces/controller_if.hpp"
#include "controllers/fire_extinguisher.hpp"

#include <chrono>
#include <thread>
#include <csignal>
#include <iostream>
#include <nettle/Writer.hpp>
#include <crate/registrar/helper.hpp>
#include <crate/networking/message_writer.hpp>
#include <crate/metrics/helper.hpp>
#include <crate/registrar/controller_v1.hpp>

#include <vector>

using namespace std::chrono_literals;

namespace {
   demu::configuration g_config;

   std::vector<demu::controller_if*> controllers;

   // Receives controller action requests and
   // attempts to route them to the controller
   class receiver : public crate::networking::message_receiver_if {
   public:
      virtual void receive_message(std::string message) override final {

         std::cout << "< received controller action request >" << std::endl;

         crate::control::action_v1_c action;
         if (!action.decode_from(message)) {
            std::cout << "Failed to decode suspected action : " << message << std::endl;
         }

         auto [ts, cid, aid, value] = action.get_data();

         // try to find the controller and send the action
         for (auto item : controllers) {
            if (item->get_controller_id() == cid) {
               item->execute_action(action);
               return;
            }
         }

         std::cout << "Unable to locate controller: " << cid << std::endl;
      }
   };
   receiver controller_command_receiver;
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
   
   // Add contollers
   //
   demu::fire_extinguisher fire_extinguisher("fire_extinguisher", env);
   controllers.push_back(&fire_extinguisher);
   crate::registrar::controller_v1_c fire_controller(
      "fire_extinguisher", 
      "A fire extinguisher",
      g_config.address,
      g_config.controller_port);

   fire_controller.add_action(crate::registrar::controller_v1_c::action_s{
      "toggle_extinguisher",
      "Toggle the fire extinguisher on and off"
   });

   std::cout << "Registering demu instance with registrar... ";

   // Submit the node for registration
   //
   crate::registrar::helper_c registrar_helper(g_config.address, g_config.http_port);
   if (registrar_helper.submit(demu) != crate::registrar::helper_c::result::SUCCESS) {
      std::cerr << "\nFailed to register node with registrar" << std::endl;
      std::exit(1);
   }

   // Submit the controller for registration
   //
   if (registrar_helper.submit(fire_controller) != crate::registrar::helper_c::result::SUCCESS) {
      std::cerr << "\nFailed to register controller with registrar" << std::endl;
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


   auto controller_command_receiver_server = new crate::networking::message_server_c(
      g_config.address, 
      g_config.controller_port, 
      &controller_command_receiver);

   if (!controller_command_receiver_server->start()) {
      std::cerr << "Failed to controller server" << std::endl;
      std::exit(1);
   }

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