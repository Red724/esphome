#include "hdc1080.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace hdc1080 {

static const char *const TAG = "hdc1080";

static const uint8_t HDC1080_ADDRESS = 0x40;  // 0b1000000 from datasheet
static const uint8_t HDC1080_CMD_CONFIGURATION = 0x02;
static const uint8_t HDC1080_CMD_TEMPERATURE = 0x00;
static const uint8_t HDC1080_CMD_HUMIDITY = 0x01;

void HDC1080Component::heater_interval_callback() {
  //ESP_LOGD(TAG, "heater_interval_callback() called");
  int i;
  for(i=0;i<1;i++){
    this->measure_and_get_temperature();
    this->measure_and_get_humidity();
  }
}

void HDC1080Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HDC1080...");

  // resolution 14bit for both humidity and temperature
  HDC1080_config_register initial_config;
  initial_config.rawData=0b00000000;

  initial_config.Heater=0;

  const uint8_t data[2] = {
      initial_config.rawData,
      0b00000000   // reserved
  };

  if (!this->write_bytes(HDC1080_CMD_CONFIGURATION, data, 2)) {
    // as instruction is same as powerup defaults (for now), interpret as warning if this fails
    ESP_LOGW(TAG, "HDC1080 initial config instruction error");
    this->status_set_warning();
    return;
  }

  //this->write_byte()

  this->set_interval(1000, std::bind(&HDC1080Component::heater_interval_callback, this));
}
void HDC1080Component::dump_config() {
  ESP_LOGCONFIG(TAG, "HDC1080:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with HDC1080 failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this->temperature_);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
}

i2c::ErrorCode HDC1080Component::measure_and_get_temperature() {
  uint16_t raw_temp;
  i2c::ErrorCode r;
  if ((r=this->write(&HDC1080_CMD_TEMPERATURE, 1)) != i2c::ERROR_OK) {
    this->status_set_warning();
    ESP_LOGW(TAG, "foo");
    return r;
  }
  delay(10);
  if ((r=this->read(reinterpret_cast<uint8_t *>(&raw_temp), 2) )!= i2c::ERROR_OK) {
    this->status_set_warning();
    ESP_LOGW(TAG, "bar");
    return r;
  }
  raw_temp = i2c::i2ctohs(raw_temp);
  this->temperature_value = raw_temp * 0.0025177f - 40.0f;  // raw * 2^-16 * 165 - 40
  return i2c::ERROR_OK;
}

i2c::ErrorCode HDC1080Component::measure_and_get_humidity() {
  uint16_t raw_humidity;
  i2c::ErrorCode r;
  if ((r=this->write(&HDC1080_CMD_HUMIDITY, 1)) != i2c::ERROR_OK) {
    this->status_set_warning();
    return r;
  }
  delay(10);
  if ((r=this->read(reinterpret_cast<uint8_t *>(&raw_humidity), 2)) != i2c::ERROR_OK) {
    this->status_set_warning();
    return r;
  }
  raw_humidity = i2c::i2ctohs(raw_humidity);
  this->humidity_value = raw_humidity * 0.001525879f;  // raw * 2^-16 * 100
  return i2c::ERROR_OK;
}

void HDC1080Component::update() {

  if(0) {//heater is off
    if (this->measure_and_get_temperature() != i2c::ERROR_OK) {
      return;
    }
    if (this->measure_and_get_humidity() != i2c::ERROR_OK) {
      return;
    }
  }

  this->temperature_->publish_state(this->temperature_value);
  this->humidity_->publish_state(this->humidity_value);
  ESP_LOGD(TAG, "Got temperature=%.1f°C humidity=%.1f%%", this->temperature_value, this->humidity_value);
  this->status_clear_warning();
}
float HDC1080Component::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace hdc1080
}  // namespace esphome
