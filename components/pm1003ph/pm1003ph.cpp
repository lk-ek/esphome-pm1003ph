#include "pm1003ph.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace pm1003ph {

static const char *const TAG = "pm1003ph";
static const uint8_t UART_REQUEST[] = {0x11, 0x02, 0x0B, 0x01, 0xE1};

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
  if (this->use_uart_ && this->parent_ != nullptr) {
    this->write_array(UART_REQUEST, sizeof(UART_REQUEST));
    delay(50);  // Give sensor time to respond

    if (this->available() >= UART_PACKET_LENGTH && this->check_uart_data_()) {
      uint16_t value = (uint16_t(this->uart_buffer_[2]) << 8) | this->uart_buffer_[3];
      if (this->pm_2_5_sensor_ != nullptr) {
        this->pm_2_5_sensor_->publish_state(value);
      }
    }
  } else if (this->pm_2_5_sensor_ != nullptr) {
    float concentration = (this->total_pulse_time_ / 30000.0f) * 1000.0f;
    this->pm_2_5_sensor_->publish_state(concentration);
    this->total_pulse_time_ = 0;  // Reset after publishing
  }
}

bool PM1003PHComponent::check_uart_data_() {
  if (this->read_array(this->uart_buffer_, UART_PACKET_LENGTH)) {
    if (this->uart_buffer_[0] != 0x16 || this->uart_buffer_[1] != 0x11) {
      ESP_LOGW(TAG, "Invalid header");
      return false;
    }
    
    // Calculate checksum
    uint8_t sum = 0;
    for (uint8_t i = 0; i < UART_PACKET_LENGTH - 1; i++) {
      sum += this->uart_buffer_[i];
    }
    
    if (sum != this->uart_buffer_[UART_PACKET_LENGTH - 1]) {
      ESP_LOGW(TAG, "Checksum mismatch");
      return false;
    }
    
    return true;
  }
  return false;
}

}  // namespace pm1003ph
}  // namespace esphome
