#pragma once

#include "esp_err.h"

#define SPI_DMA_CHAN 1
#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 22

esp_err_t sdcard_init(void);