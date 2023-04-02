#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace hdc1080 {

typedef union {
  uint8_t rawData;
  struct {
    uint8_t HumidityMeasurementResolution : 2;
    uint8_t TemperatureMeasurementResolution : 1;
    uint8_t BatteryStatus : 1;
    uint8_t ModeOfAcquisition : 1;
    uint8_t Heater : 1;
    uint8_t ReservedAgain : 1;
    uint8_t SoftwareReset : 1;
  };
} HDC1080_config_register;

class HDC1080Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_temperature(sensor::Sensor *temperature) { temperature_ = temperature; }
  void set_humidity(sensor::Sensor *humidity) { humidity_ = humidity; }

  /// Setup the sensor and check for connection.
  void setup() override;
  void dump_config() override;
  /// Retrieve the latest sensor values. This operation takes approximately 16ms.
  void update() override;

  float get_setup_priority() const override;

 protected:
  sensor::Sensor *temperature_{nullptr};
  sensor::Sensor *humidity_{nullptr};
  float temperature_value;
  float humidity_value;

  i2c::ErrorCode measure_and_get_temperature();
  i2c::ErrorCode measure_and_get_humidity();
};

}  // namespace hdc1080
}  // namespace esphome
