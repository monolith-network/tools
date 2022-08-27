#include "environment.hpp"
#include <libutil/random/generator.hpp>

namespace demu {

void environment::configure(std::vector<environment_details> details) {
   _details = details;

   for(auto &detail : details) {
      switch (detail) {
         case environment_details::DRY:
            range.humidity_high = 30;
            is_dry = true;
          break;
         case environment_details::WET: 
            range.humidity_low = 60;
            is_dry = false;
         break;
         case environment_details::HIGH_TRAFFIC: 
            range.activity_prob = 8;
         break;
         case environment_details::MEDIUM_TRAFFIC: 
            range.activity_prob = 5;
         break;
         case environment_details::LOW_TRAFFIC: 
            range.activity_prob = 2;
         break;
         case environment_details::HOT: 
            range.temp_low = 80.0;
            if (range.temp_low > range.temp_high) {
               range.temp_low = range.temp_high;
            }
            is_hot = true;
         break;
         case environment_details::COLD: 
            range.temp_high = 70.0;
            is_hot = false;
         break;
         case environment_details::TEMPERATE: 
            range.temp_low = 60.0;
            range.temp_high = 80.0;
         break;
      }
   }

   auto dbl_rand = libutil::random::generate_random_c<double>();
   auto u32_rand = libutil::random::generate_random_c<uint32_t>();
   auto u8_rand = libutil::random::generate_random_c<uint8_t>();

   // Set the base values of everything
   //
   level.temperature = dbl_rand.get_floating_point_range(range.temp_low, range.temp_high);
   level.humidity = dbl_rand.get_floating_point_range(range.humidity_low, range.humidity_high);
   level.pressure = dbl_rand.get_floating_point_range(range.pressure_low, range.pressure_high);
   level.light = u32_rand.get_range(range.light_low, range.light_high);
   level.flames = u8_rand.get_range(range.flames_low, range.flames_high);
   level.activity = u8_rand.get_range(0, 10) <= range.activity_prob;
}

double environment::poll_temperature()
{
   update();
   return level.temperature;
}

double environment::poll_humidity()
{
   update();
   return level.humidity;
}

uint32_t environment::poll_air_pressure()
{
   update();
   return level.pressure;
}

uint8_t environment::poll_light()
{
   update();
   return level.light;
}

uint8_t environment::poll_flames()
{
   update();

   if (has_fire) {
      return level.flames;
   } else {
      return 0.0;
   }
}

bool environment::is_active()
{
   update();
   return level.activity;
}

void environment::update()
{
   auto dbl_rand = libutil::random::generate_random_c<double>();
   auto u32_rand = libutil::random::generate_random_c<uint32_t>();
   auto u8_rand = libutil::random::generate_random_c<uint8_t>();

   //    Check to see if we should start a fire
   //
   if (!has_fire) {
      if (is_hot && is_dry && level.temperature > 90.0) {
         has_fire = u8_rand.get_range(0, 1);
      } else {

         // 1 in 100 chance of a random fire starting
         uint8_t r = u8_rand.get_range(0, 100);
         if (r == 50) {
            has_fire = true;
         }
      }
   } else {
      has_fire = u8_rand.get_range(0, 1);
   }
   level.flames = u8_rand.get_range(range.flames_low, range.flames_high);

   //    Temp
   //
   if (has_fire) {
      level.temperature = range.temp_high;
   } else {
      uint8_t r = u8_rand.get_range(0, 2);
      double val = dbl_rand.get_floating_point_range(0.0, 0.5);
      if (r == 0) {
         // No temperature change
      } else if (r == 1) {
         level.temperature += val;
      } else {
         level.temperature -= val;
      }
      if (level.temperature < range.temp_low) {
         level.temperature = range.temp_low;
      }
      if (level.temperature > range.temp_high) {
         level.temperature = range.temp_high;
      }
   }

   // Humidity
   //
   {
      uint8_t r = u8_rand.get_range(0, 2);
      double val = dbl_rand.get_floating_point_range(0.0, 0.5);
      if (r == 0) {
         // No change
      } else if (r == 1) {
         level.humidity += val;
      } else {
         level.humidity -= val;
      }
      if (level.humidity > range.humidity_high) {
         level.humidity = range.humidity_high;
      }
      if (level.humidity < range.humidity_low) {
         level.humidity = range.humidity_low;
      }
   }

   // Pressure
   //
   {
      uint8_t r = u8_rand.get_range(0, 2);
      double val = dbl_rand.get_floating_point_range(0.0, 0.5);
      if (r == 0) {
         // No change
      } else if (r == 1) {
         level.pressure += val;
      } else {
         level.pressure -= val;
      }
      if (level.pressure > range.pressure_high) {
         level.pressure = range.pressure_high;
      }
      if (level.pressure < range.pressure_low) {
         level.pressure = range.pressure_low;
      }
   }

   // Light
   //
   {
      if (has_fire) {
         level.light = range.light_high;
      } else {
         uint8_t r = u8_rand.get_range(0, 2);
         uint8_t val = u8_rand.get_range(0, 1);
         if (r == 0) {
            // No change
         } else if (r == 1) {
            level.light += val;
         } else {
            level.light -= val;
         }
      }
      if (level.light > range.light_high) {
         level.light = range.light_high;
      }
      if (level.light < range.light_low) {
         level.light = range.light_low;
      }
   }

   //    Activity
   //
   level.activity = u8_rand.get_range(0, 10) <= range.activity_prob;
}

} // namespace demu
