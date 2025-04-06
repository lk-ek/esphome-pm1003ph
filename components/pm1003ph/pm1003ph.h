#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace pm1003ph {

class PM1003PHComponent : public PollingComponent {
 public:
  void set_pm_2_5_sensor(sensor::Sensor *pm_2_5_sensor) { this->pm_2_5_sensor_ = pm_2_5_sensor; }
  void set_binary_sensor(binary_sensor::BinarySensor *binary_sensor) { this->binary_sensor_ = binary_sensor; }

  void setup() override;
  void update() override;

 protected:
  sensor::Sensor *pm_2_5_sensor_{nullptr};
  binary_sensor::BinarySensor *binary_sensor_{nullptr};  // Use external binary_sensor

  unsigned long pulse_start_time_{0};
  unsigned long total_pulse_time_{0};
};

}  // namespace pm1003ph
}  // namespace esphome
