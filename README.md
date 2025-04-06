# PM1003PH Component for ESPHome

This repository contains the `pm1003ph` component for ESPHome, which supports the PM1003PH particulate matter sensor, included for example in Philips AC0830 air purifiers.

Sensor: https://en.gassensor.com.cn/ParticulateMatterSensor/info_itemid_104.html
Datasheet: https://agelectronica.lat/pdfs/textos/P/PM1003PH.PDF


## Features
- Measures PM2.5 concentration using a binary sensor for PWM signal input.
- Publishes PM2.5 data to Home Assistant.

## Installation
To use this component, copy the `pm1003ph` directory into your ESPHome project's `custom_components` folder.

## Configuration Example
```yaml
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
