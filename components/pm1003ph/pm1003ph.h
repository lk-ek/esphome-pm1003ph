#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_UART
#include "esphome/components/uart/uart.h"
#endif

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

namespace esphome {
namespace pm1003ph {

static const uint8_t UART_PACKET_LENGTH = 20;

class PM1003PHComponent : public PollingComponent
#ifdef USE_UART
    , public uart::UARTDevice
#endif
{
 public:
#ifdef USE_UART
  PM1003PHComponent() : UARTDevice(nullptr) {}
#else
  PM1003PHComponent() {}
#endif
  
  void set_pm_2_5_sensor(sensor::Sensor *pm_2_5_sensor) { this->pm_2_5_sensor_ = pm_2_5_sensor; }

#ifdef USE_BINARY_SENSOR
  void set_binary_sensor(binary_sensor::BinarySensor *binary_sensor) { this->binary_sensor_ = binary_sensor; }
#endif

  void setup() override;
  void update() override;

#ifdef USE_UART
  void set_use_uart(bool use_uart) { use_uart_ = use_uart; }
  void set_uart_parent(uart::UARTComponent *parent) {
    UARTDevice::set_uart_parent(parent);
    this->use_uart_ = (parent != nullptr);
  }
#endif

 protected:
  sensor::Sensor *pm_2_5_sensor_{nullptr};
#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *binary_sensor_{nullptr};
#endif

  // Only include these variables when UART is enabled
#ifdef USE_UART
  bool use_uart_{false};
  uint8_t uart_buffer_[UART_PACKET_LENGTH];
  bool check_uart_data_();
#endif

  unsigned long pulse_start_time_{0};
  unsigned long total_pulse_time_{0};
};

}  // namespace pm1003ph
}  // namespace esphome
