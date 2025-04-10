import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_PM_2_5,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_PM25,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
    ICON_BLUR,
)

DEPENDENCIES = ["uart"]

pm1003ph_ns = cg.esphome_ns.namespace("pm1003ph")
PM1003PHComponent = pm1003ph_ns.class_("PM1003PHComponent", cg.PollingComponent)

CONF_BINARY_SENSOR = "binary_sensor"
CONF_UART_ID = "uart_id"
CONF_USE_UART = "use_uart"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PM1003PHComponent),
            cv.Optional(CONF_PM_2_5): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
                icon=ICON_BLUR,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,  
                device_class=DEVICE_CLASS_PM25,
            ),
            cv.Optional(CONF_UART_ID): cv.use_id(uart.UARTComponent),
            cv.Optional(CONF_USE_UART, default=False): cv.boolean,
            cv.Optional(CONF_BINARY_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(cv.polling_component_schema("30s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    
    if CONF_UART_ID in config:
        uart_component = await cg.get_variable(config[CONF_UART_ID])
        cg.add(var.set_uart_parent(uart_component))
    
    await cg.register_component(var, config)
    
    if CONF_BINARY_SENSOR in config:
        cg.add_define("USE_BINARY_SENSOR")  # This is the correct way to enable binary sensor
        binary_sensor_ = await cg.get_variable(config[CONF_BINARY_SENSOR])
        cg.add(var.set_binary_sensor(binary_sensor_))

    if CONF_PM_2_5 in config:
        sens = await sensor.new_sensor(config[CONF_PM_2_5])
        cg.add(var.set_pm_2_5_sensor(sens))
    
    if CONF_USE_UART in config:
        cg.add(var.set_use_uart(config[CONF_USE_UART]))
