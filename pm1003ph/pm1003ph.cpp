#include "pm1003ph.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace pm1003ph {

static const char *const TAG = "pm1003ph";

void PM1003PHComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PM1003PH...");
  this->binary_sensor_->add_on_state_callback([this](bool state) {
    unsigned long now = millis();  // Use esphome::millis()
    if (state) {
      // Binary sensor is pressed (on)
      if (this->pulse_start_time_ > 0) {
        this->total_pulse_time_ += now - this->pulse_start_time_;
      }
      this->pulse_start_time_ = now;
    } else {
      // Binary sensor is released (off)
      this->pulse_start_time_ = now;
    }
  });
}

void PM1003PHComponent::update() {
  if (this->pm_2_5_sensor_ != nullptr) {
    float concentration = (this->total_pulse_time_ / 30000.0f) * 1000.0f;
    this->pm_2_5_sensor_->publish_state(concentration);
    this->total_pulse_time_ = 0;  // Reset after publishing
  }
}

}  // namespace pm1003ph
}  // namespace esphome
