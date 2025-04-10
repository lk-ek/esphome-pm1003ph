# PM1003PH Component for ESPHome

This repository contains the `pm1003ph` component for ESPHome, which supports
the PM1003PH particulate matter sensor, included for example in Philips AC0830
air purifiers.


The sensor outputs a PWM signal, the total pulse widths over 30s proportionally
represent the PM2.5 concentration. Or, the "1-10ug/qm particulate
concentration" if the datasheet is to be taken seriously.

Sensor: https://en.gassensor.com.cn/ParticulateMatterSensor/info_itemid_104.html

Datasheet: https://agelectronica.lat/pdfs/textos/P/PM1003PH.PDF

## Caveats
The sensor runs on 5V and the UART and PWM output levels are 4.5V. You need to
use a voltage divider if you plan to connect your ESP. Sensor-to-ESP 1K Ohm and
then from the 1K Ohm resistor another 2.2K Ohm to ground worked for me on both
the PWM as on the sensors TX line, which must be connected to the ESPs RX. The
ESPs TX can be directly connected to the sensors RX, it seems to accept the ESPs
3.3V level as input.

## Features
- Measures PM2.5 concentration using a binary sensor for PWM signal input.
- Publishes PM2.5 data to Home Assistant.
- Can use either the PWM output of the sensor or the UART output of the sensor.

## Installation
To use this component, either copy the `pm1003ph` directory into your ESPHome
project's `custom_components` folder or use the following external_components
configuration:

```yaml

external_components:
  - source:
      type: git
      url: https://github.com/lk-ek/esphome-pm1003ph
      ref: main 



```

## Configuration Example (PWM Output)
```yaml

external_components:
  - source:
      type: git
      url: https://github.com/lk-ek/esphome-pm1003ph
      ref: main 

binary_sensor:
  - platform: gpio
    pin: GPIO4
    id: pm1003ph_pwm_signal
    internal: true

sensor:
  - platform: pm1003ph
    binary_sensor: pm1003ph_pwm_signal
    pm_2_5:
      name: "PM1003PH PM2.5"
```

## Configuration Example (UART Output)
```yaml

external_components:
  - source:
      type: git
      url: https://github.com/lk-ek/esphome-pm1003ph
      ref: main 

uart:
  id: uart_bus
  tx_pin: GPIO43
  rx_pin: GPIO44
  baud_rate: 9600

sensor:
  - platform: pm1003ph
    use_uart: true
    uart_id: uart_bus
    pm_2_5:
      name: "PM1003PH PM2.5"
```
