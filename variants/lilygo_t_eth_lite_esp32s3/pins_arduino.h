#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

// UART0 used by the external USB-UART adapter.
static const uint8_t TX = 43;
static const uint8_t RX = 44;

// Default pins for an external I2C bus.
static const uint8_t SDA = 38;
static const uint8_t SCL = 39;

// Default SPI pins are connected to the onboard W5500 Ethernet controller.
static const uint8_t SS = 9;
static const uint8_t MOSI = 12;
static const uint8_t MISO = 11;
static const uint8_t SCK = 10;

// Onboard W5500 Ethernet controller.
#define ETH_MISO_PIN 11
#define ETH_MOSI_PIN 12
#define ETH_SCLK_PIN 10
#define ETH_CS_PIN   9
#define ETH_INT_PIN  13
#define ETH_RST_PIN  14
#define ETH_ADDR     1

// Onboard microSD card slot, using a separate SPI bus.
#define SD_MISO_PIN 5
#define SD_MOSI_PIN 6
#define SD_SCLK_PIN 7
#define SD_CS_PIN   42

// Infrared filter input used by the T-ETH-Lite family.
#define IR_FILTER_NUM 46

#endif /* Pins_Arduino_h */
