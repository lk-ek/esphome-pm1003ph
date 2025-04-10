#include "pm1003ph.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <algorithm>  // Add for std::min

namespace esphome {
namespace pm1003ph {

static const char *const TAG = "pm1003ph";
static const uint8_t UART_REQUEST[] = {0x11, 0x02, 0x0B, 0x01, 0xE1};

void PM1003PHComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PM1003PH...");
#ifdef USE_BINARY_SENSOR
#ifdef USE_UART
  if (!this->use_uart_ && this->binary_sensor_ == nullptr) {
#else
  if (this->binary_sensor_ == nullptr) {
#endif
    ESP_LOGE(TAG, "Binary sensor is required for PWM mode!");
    this->mark_failed();
    return;
  }
  if (this->binary_sensor_ != nullptr) {  // always setup PWM if sensor exists
    this->pulse_history_.reserve(MAX_PULSES);
    this->measurement_start_time_ = millis();  // Start measuring from setup
    this->binary_sensor_->add_on_state_callback([this](bool state) {
      uint32_t now = millis();
      if (!state) {  // OFF pulse starts
        this->pulse_start_time_ = now;
      } else {  // OFF pulse ends
        if (this->pulse_start_time_ > 0) {
          uint32_t duration = now - this->pulse_start_time_;
          this->pulse_history_.push_back({now, duration});
          if (this->pulse_history_.size() > MAX_PULSES) {
            this->pulse_history_.erase(this->pulse_history_.begin());
          }
          ESP_LOGV(TAG, "PWM OFF Pulse: %u ms", duration);
        }
      }
    });
  }
#endif
}

#ifdef USE_BINARY_SENSOR
void PM1003PHComponent::cleanup_old_pulses_(uint32_t now) {
  while (!this->pulse_history_.empty() && 
         (now - this->pulse_history_.front().timestamp) > HISTORY_WINDOW_MS) {
    this->pulse_history_.erase(this->pulse_history_.begin());
  }
}

optional<float> PM1003PHComponent::calculate_pwm_concentration_() {
  uint32_t now = millis();
  this->cleanup_old_pulses_(now);

  if (this->pulse_history_.empty()) {
    return {};  // Return empty optional
  }

  // Calculate window start time (30 seconds ago)
  uint32_t window_start = now - PWM_WINDOW_MS;
  
  // Calculate total OFF time in the last 30 seconds
  uint32_t total_off_time = 0;
  for (const auto &pulse : this->pulse_history_) {
    if (pulse.timestamp >= window_start) {
      total_off_time += pulse.duration;
    }
  }

  // Use absolute time since measurement start
  uint32_t elapsed = now - this->measurement_start_time_;
  if (elapsed < PWM_WINDOW_MS) {
    ESP_LOGW(TAG, "Not enough data yet: %u ms of %u ms", elapsed, PWM_WINDOW_MS);
    return {};  // Return empty optional
  }

  float duty_ratio = total_off_time / float(PWM_WINDOW_MS);
  float concentration = duty_ratio * 1000.0f;
  
  ESP_LOGD(TAG, "PWM - Last 30s: OFF time: %u ms, Duty: %.3f%%, Pulses: %u, Conc: %.1f µg/m³", 
           total_off_time, duty_ratio * 100, this->pulse_history_.size(), concentration);
  
  return concentration;
}
#endif

void PM1003PHComponent::update() {
  float uart_concentration = 0;
  float pwm_concentration = 0;
  bool uart_valid = false;
  optional<float> pwm_result{};  // Declare at the beginning of the function

#ifdef USE_UART
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
      // Value is in bytes 5,6 (3rd and 4th after 16 11 0B header)
      uint16_t raw_value = encode_uint16(this->uart_buffer_[5], this->uart_buffer_[6]);
      ESP_LOGV(TAG, "UART bytes: [%02X %02X %02X] %02X %02X %02X %02X", 
               this->uart_buffer_[0], this->uart_buffer_[1], this->uart_buffer_[2],
               this->uart_buffer_[3], this->uart_buffer_[4], this->uart_buffer_[5], this->uart_buffer_[6]);
      uart_concentration = raw_value;
      uart_valid = true;
      ESP_LOGD(TAG, "UART - Raw: %d (PWM duty %.3f%%)", raw_value, raw_value/10.0f);
    }
  }
#endif

  // Handle PWM reading
#ifdef USE_BINARY_SENSOR
  if (this->binary_sensor_ != nullptr) {
    pwm_result = this->calculate_pwm_concentration_();
    if (pwm_result.has_value()) {
      pwm_concentration = *pwm_result;
    }
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

  // Publish value based on available method
  if (this->pm_2_5_sensor_ != nullptr) {
#ifdef USE_UART
    if (uart_valid) {
      this->pm_2_5_sensor_->publish_state(uart_concentration);
      return;
    }
#endif

#ifdef USE_BINARY_SENSOR
    if (this->binary_sensor_ != nullptr && pwm_result.has_value()) {
      this->pm_2_5_sensor_->publish_state(*pwm_result);
    }
#endif
  }
}

#ifdef USE_UART
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
#endif

}  // namespace pm1003ph
}  // namespace esphome
