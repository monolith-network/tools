#include "config.hpp"

#include <iostream>
#include <toml++/toml.h>

#include "sensors/temperature.hpp"
#include "sensors/motion.hpp"
#include "sensors/humidity.hpp"
#include "sensors/flame.hpp"
#include "sensors/air_pressure.hpp"
#include "sensors/light.hpp"

namespace demu {
   
std::optional<configuration> load_config(const std::string& file, environment &env) {

   toml::table tbl;
   try {
      tbl = toml::parse_file(file);
   } catch (const toml::parse_error& err) {
      std::cerr << "Unable to parse file : " 
         << *err.source().path 
         << ". Description: " 
         << err.description() 
         << " (" << err.source().begin << ")\n";
      return {};
   }

   configuration config;

   std::optional<std::string> device_name = tbl["demu"]["device_name"].value<std::string>();
   if (device_name.has_value()) {
      config.device_name = *device_name;
   } else {
      std::cout << "Missing config for 'device name'\n";
      return {}; 
   }

   std::optional<std::string> location_name = tbl["demu"]["location_name"].value<std::string>();
   if (device_name.has_value()) {
      config.location_name = *location_name;
   } else {
      std::cout << "Missing config for 'location name'\n";
      return {}; 
   }

   std::optional<uint32_t> controller_port = tbl["demu"]["controller_port"].value<uint32_t>();
   if (controller_port.has_value()) {
      config.controller_port = *controller_port;
   } else {
      std::cout << "Missing config for 'controller port'\n";
      return {}; 
   }

   std::optional<std::string> monoloth_address = tbl["monolith"]["address"].value<std::string>();
   if (monoloth_address.has_value()) {
      config.address = *monoloth_address;
   } else {
      std::cout << "Missing config for 'address'\n";
      return {}; 
   }
   
   std::optional<uint32_t> http_port = tbl["monolith"]["http_port"].value<uint32_t>();
   if (http_port.has_value()) {
      config.http_port = *http_port;
   } else {
      std::cout << "Missing config for 'http port'\n";
      return {}; 
   }
   
   std::optional<uint32_t> metric_submission_port = tbl["monolith"]["metric_submission_port"].value<uint32_t>();
   if (metric_submission_port.has_value()) {
      config.metric_submission_port = *metric_submission_port;
   } else {
      std::cout << "Missing config for 'http port'\n";
      return {}; 
   }

   // Sensor details
   //
   auto sensor_array = tbl["demu"]["sensors"];
   if (sensor_array.is_array() ) {
      toml::array* arr = sensor_array.as_array();
      for(auto &item : *arr) {
         if (!item.is_string()) {
            std::cerr << "Invalid type in sensor array" << std::endl;
            return {};
         }

         auto sensor_type_string = (*item.as_string()).get();
         if (sensor_type_string == "temperature") {
            config.sensors.push_back(
               new temperature(config.device_name, env)
            );
         }
         else if (sensor_type_string == "motion") {
            config.sensors.push_back(
               new motion(config.device_name, env)
            );
         }
         else if (sensor_type_string == "humidity") {
            config.sensors.push_back(
               new humidity(config.device_name, env)
            );
         }
         else if (sensor_type_string == "flame") {
            config.sensors.push_back(
               new flame(config.device_name, env)
            );
         }
         else if (sensor_type_string == "air_pressure") {
            config.sensors.push_back(
               new air_pressure(config.device_name, env)
            );
         }
         else if (sensor_type_string == "light") {
            config.sensors.push_back(
               new light(config.device_name, env)
            );
         }
         else {
            std::cerr << "Unknown sensor type : " << sensor_type_string << std::endl;
            return {};
         }
      }
   }

   // Env details
   //
   auto env_array = tbl["demu"]["environment_detail"];
   if (env_array.is_array() ) {
      toml::array* arr = env_array.as_array();
      for(auto &item : *arr) {
         if (!item.is_string()) {
            std::cerr << "Invalid type in env array" << std::endl;
            return {};
         }

         auto env_type = (*item.as_string()).get();
         if (env_type == "dry") {
            config.env_details.push_back(
               environment_details::DRY
            );
         }
         else if (env_type == "wet") {
            config.env_details.push_back(
               environment_details::WET
            );
         }
         else if (env_type == "high_traffic") {
            config.env_details.push_back(
               environment_details::HIGH_TRAFFIC
            );
         }
         else if (env_type == "medium_traffic") {
            config.env_details.push_back(
               environment_details::MEDIUM_TRAFFIC
            );
         }
         else if (env_type == "low_traffic") {
            config.env_details.push_back(
               environment_details::LOW_TRAFFIC
            );
         }
         else if (env_type == "hot") {
            config.env_details.push_back(
               environment_details::HOT
            );
         }
         else if (env_type == "cold") {
            config.env_details.push_back(
               environment_details::COLD
            );
         }
         else if (env_type == "temperate") {
            config.env_details.push_back(
               environment_details::TEMPERATE
            );
         }
         else {
            std::cerr << "Unknown environment detail : " << env_type << std::endl;
            return {};
         }
      }
   }
   return {config};
}

}