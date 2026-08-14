#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "soc/soc_caps.h"

// USB-UART bridge used for programming and the Arduino serial monitor.
static const uint8_t TX = 37;
static const uint8_t RX = 38;

// On-board I2C bus.
static const uint8_t SDA = 7;
static const uint8_t SCL = 8;

// On-board 100M Ethernet PHY: IP101GRI, RMII.
#define ETH_PHY_TYPE    ETH_PHY_IP101
#define ETH_PHY_ADDR    1
#define ETH_PHY_MDC     31
#define ETH_PHY_MDIO    52
#define ETH_PHY_POWER   51
#define ETH_RMII_TX_EN  49
#define ETH_RMII_TX0    34
#define ETH_RMII_TX1    35
#define ETH_RMII_RX0    29
#define ETH_RMII_RX1_EN 30
#define ETH_RMII_CRS_DV 28
#define ETH_RMII_CLK    50
#define ETH_CLK_MODE    EMAC_CLK_EXT_IN

// TF card slot, SDMMC.
#define BOARD_HAS_SDMMC
#define BOARD_SDMMC_SLOT           0
#define BOARD_SDMMC_POWER_CHANNEL  4
#define BOARD_SDMMC_POWER_PIN      45
#define BOARD_SDMMC_POWER_ON_LEVEL LOW

// GPIO39-48 are supplied by the ESP32-P4 peripheral I/O LDO.
#define BOARD_PERIMAN_IO_LDO_AUTO        1
#define BOARD_PERIMAN_IO_LDO0_CHANNEL    4
#define BOARD_PERIMAN_IO_LDO0_GPIO_MIN   39
#define BOARD_PERIMAN_IO_LDO0_GPIO_MAX   48
#define BOARD_PERIMAN_IO_LDO0_VOLTAGE_MV 3300

// ES8311 codec and speaker amplifier.
#define BOARD_HAS_ES8311
#define I2S_MCLK  13
#define I2S_BCLK  12
#define I2S_LRCLK 10
#define I2S_DOUT  11
#define I2S_DIN   9
#define PA_POWER  53

#endif /* Pins_Arduino_h */
