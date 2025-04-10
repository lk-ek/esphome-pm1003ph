#include "pm1003ph.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace pm1003ph {

static const char *const TAG = "pm1003ph";
static const uint8_t UART_REQUEST[] = {0x11, 0x02, 0x0B, 0x01, 0xE1};

void PM1003PHComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PM1003PH...");
#ifdef USE_BINARY_SENSOR
  if (!this->use_uart_ && this->binary_sensor_ == nullptr) {
    ESP_LOGE(TAG, "Binary sensor is required for PWM mode!");
    this->mark_failed();
    return;
  }
  if (this->binary_sensor_ != nullptr) {  // Remove UART check to always set up PWM if sensor exists
    this->binary_sensor_->add_on_state_callback([this](bool state) {
      unsigned long now = millis();
      if (state) {
        this->pulse_start_time_ = now;
      } else {
        if (this->pulse_start_time_ > 0) {
          this->total_pulse_time_ += now - this->pulse_start_time_;
          ESP_LOGV(TAG, "PWM Pulse width: %lu ms", now - this->pulse_start_time_);
        }
      }
    });
  }
#endif
}

void PM1003PHComponent::update() {
  float uart_concentration = 0;
  float pwm_concentration = 0;
  bool uart_valid = false;

  // Handle UART reading
  if (this->parent_ != nullptr) {
    ESP_LOGD(TAG, "Sending UART request");
    this->write_array(UART_REQUEST, sizeof(UART_REQUEST));
    
    uint32_t start_time = millis();
    while (this->available() < UART_PACKET_LENGTH) {
      if (millis() - start_time > 50) {
        ESP_LOGW(TAG, "Timeout waiting for response");
        break;
      }
      yield();
    }

    if (this->check_uart_data_()) {
      // According to datasheet: The value(n) = DF3*256+DF4 (offset 3,4 after header)
      uint16_t raw_value = (uint16_t(this->uart_buffer_[6]) << 8) | this->uart_buffer_[7];
      float duty_ratio = raw_value / 1000.0f;
      uart_concentration = duty_ratio * 1000.0f;
      uart_valid = true;
      ESP_LOGD(TAG, "UART - Raw: %d, Duty: %.3f, Conc: %.1f", raw_value, duty_ratio, uart_concentration);
    }
  }

  // Handle PWM reading
#ifdef USE_BINARY_SENSOR
  if (this->binary_sensor_ != nullptr) {
    // Calculate duty cycle over the full update interval
    float interval_ms = this->get_update_interval() * 1000;  // convert to milliseconds
    float duty_ratio = this->total_pulse_time_ / interval_ms;
    pwm_concentration = duty_ratio * 1000.0f;
    
    ESP_LOGD(TAG, "PWM - Total ON time: %lu ms, Interval: %.0f ms, Duty: %.3f, Conc: %.1f", 
             this->total_pulse_time_, interval_ms, duty_ratio, pwm_concentration);
    
    this->total_pulse_time_ = 0;  // Reset after reading
  }
#endif

  // Log comparison if both methods available
#ifdef USE_BINARY_SENSOR
  if (uart_valid && this->binary_sensor_ != nullptr) {
    float diff = uart_concentration - pwm_concentration;
    ESP_LOGD(TAG, "Comparison - UART: %.1f, PWM: %.1f, Diff: %.1f", 
             uart_concentration, pwm_concentration, diff);
  }
#endif

  // Publish UART value if valid, otherwise use PWM value
  if (this->pm_2_5_sensor_ != nullptr) {
    if (uart_valid) {
      this->pm_2_5_sensor_->publish_state(uart_concentration);
#ifdef USE_BINARY_SENSOR
    } else if (this->binary_sensor_ != nullptr) {
      this->pm_2_5_sensor_->publish_state(pwm_concentration);
#endif
    }
  }
}

bool PM1003PHComponent::check_uart_data_() {
  if (this->read_array(this->uart_buffer_, UART_PACKET_LENGTH)) {
    if (this->uart_buffer_[0] != 0x16 || this->uart_buffer_[1] != 0x11 || this->uart_buffer_[2] != 0x0B) {
      ESP_LOGW(TAG, "Invalid header [%02X %02X %02X]", this->uart_buffer_[0], 
               this->uart_buffer_[1], this->uart_buffer_[2]);
      return false;
    }
    
    // Log the full received data for debugging
    ESP_LOGV(TAG, "Received data:");
    for (int i = 0; i < UART_PACKET_LENGTH; i++) {
      ESP_LOGV(TAG, "byte[%d] = 0x%02X", i, this->uart_buffer_[i]);
    }
    
    return true;  // Temporarily disable checksum check for debugging
  }
  return false;
}

}  // namespace pm1003ph
}  // namespace esphome
