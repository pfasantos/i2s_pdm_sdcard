#pragma once

#define REC_TIME_MS 5000
#define FSYNC_DELAY_BLOCKS 8

static void vTaskReadData(void *pvParameters);
static void vTaskStoreData(void *pvParameters);
