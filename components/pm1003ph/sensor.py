import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor
from esphome.const import (
    CONF_ID,
    CONF_PM_2_5,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_PM25,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
    ICON_BLUR,
)

pm1003ph_ns = cg.esphome_ns.namespace("pm1003ph")
PM1003PHComponent = pm1003ph_ns.class_("PM1003PHComponent", cg.PollingComponent)

CONF_BINARY_SENSOR = "binary_sensor"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(PM1003PHComponent),
        cv.Required(CONF_BINARY_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_PM_2_5): sensor.sensor_schema(
            unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
            icon=ICON_BLUR
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,  
            device_class=DEVICE_CLASS_PM25,
        ),
    }
).extend(cv.polling_component_schema("30s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    binary_sensor_ = await cg.get_variable(config[CONF_BINARY_SENSOR])
    cg.add(var.set_binary_sensor(binary_sensor_))

    if CONF_PM_2_5 in config:
        sens = await sensor.new_sensor(config[CONF_PM_2_5])
        cg.add(var.set_pm_2_5_sensor(sens))
