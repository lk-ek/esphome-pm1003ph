#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

namespace esphome {
namespace pm1003ph {

static const uint8_t UART_PACKET_LENGTH = 20;

class PM1003PHComponent : public PollingComponent, public uart::UARTDevice {
 public:
  PM1003PHComponent() : UARTDevice(nullptr) {}
  
  void set_pm_2_5_sensor(sensor::Sensor *pm_2_5_sensor) { this->pm_2_5_sensor_ = pm_2_5_sensor; }

#ifdef USE_BINARY_SENSOR
  void set_binary_sensor(binary_sensor::BinarySensor *binary_sensor) { this->binary_sensor_ = binary_sensor; }
#endif

  void setup() override;
  void update() override;

  void set_use_uart(bool use_uart) { use_uart_ = use_uart; }
  void set_uart_parent(uart::UARTComponent *parent) {
    UARTDevice::set_uart_parent(parent);  // Call parent class method
    this->use_uart_ = (parent != nullptr);
  }

 protected:
  sensor::Sensor *pm_2_5_sensor_{nullptr};
#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *binary_sensor_{nullptr};
#endif
  unsigned long pulse_start_time_{0};
  unsigned long total_pulse_time_{0};

  bool use_uart_{false};
  uint8_t uart_buffer_[UART_PACKET_LENGTH];
  bool check_uart_data_();
};

}  // namespace pm1003ph
}  // namespace esphome
