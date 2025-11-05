#include "i2s_driver.h"
#include "sd_driver.h"
#include "main.h"
#include <stdio.h>
#include <sys/param.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include <errno.h>

#include "driver/i2s_std.h"
#include "driver/i2s_pdm.h"
#include "driver/gpio.h"

FILE* audio_file = NULL;

char rx_buffer[BUF_SIZE];
char tx_buffer[BUF_SIZE];

int block_count = 0;

QueueHandle_t xQueueData;
TaskHandle_t xTaskReadDataHandle;
TaskHandle_t xTaskStoreDataHandle;

static const char *MAIN_TAG = "MAIN";
static const char *I2S_TAG = "I2S";

static void vTaskReadData(void *pvParameters)
{
    size_t bytes_to_read = BUF_SIZE;
    size_t bytes_read;

    //inicia canal i2s
    i2s_channel_enable(rx_handle);

    TickType_t start_time = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(REC_TIME_MS))
    {
        if (i2s_channel_read(rx_handle, (void *)rx_buffer, bytes_to_read, &bytes_read, portMAX_DELAY) == ESP_OK)
        {
            xQueueSend(xQueueData, &rx_buffer, portMAX_DELAY); //aguarda buffer encher e manda pra fila
        }
        vTaskDelay(1);
    }

    i2s_channel_disable(rx_handle);
    ESP_LOGI(I2S_TAG, "Aquisição encerrada");
    vTaskDelete(NULL);
}

static void vTaskStoreData(void *pvParameters)
{
    while (1)
    {
        if (xQueueReceive(xQueueData, &tx_buffer, portMAX_DELAY) == pdTRUE) {
            if (audio_file) {
                fwrite(tx_buffer, 1, BUF_SIZE, audio_file);
                block_count++;

                if (block_count >= FSYNC_DELAY_BLOCKS) {
                    fflush(audio_file);
                    fsync(fileno(audio_file));
                    block_count = 0;
                }
            }
        }
    }

    vTaskDelete(NULL);
}

void app_main(void)
{
    i2s_pdm_init();
    sdcard_init();
    audio_file = fopen(MOUNT_POINT "/audio.raw", "wb");

    xQueueData = xQueueCreate(DMA_BUF_NUM, BUF_SIZE*sizeof(char));
    if(xQueueData == NULL){ // testar se a criacao da fila falhou
        ESP_LOGE(MAIN_TAG, "Falha em criar fila de dados");
        while(1);
    }

    xTaskCreate(vTaskReadData,"taskREAD", 4096, NULL, 3, NULL);
    xTaskCreate(vTaskStoreData,"taskSTORE", 4096, NULL, 3, NULL); 

}