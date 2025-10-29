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

char in_buffer[BUF_SIZE];
char rx_buffer[BUF_SIZE];

int block_count = 0;

QueueHandle_t xQueueData;
TaskHandle_t xTaskReadDataHandle;
TaskHandle_t xTaskStoreDataHandle;

static const char *MAIN_TAG = "MAIN";
static const char *I2S_TAG = "I2S";

static void vTaskReadData(void * pvParameters)
{
    size_t bytes_to_read = BUF_SIZE; // quantidades de bytes para ler
    size_t bytes_read; // quantidade de bytes lidos

    TickType_t start_time = xTaskGetTickCount();

    // iniciar o canal i2s
    i2s_channel_enable(rx_handle);

    while ((xTaskGetTickCount() - start_time)< pdMS_TO_TICKS(REC_TIME_MS)) {
        if (i2s_channel_read(rx_handle, (void*) in_buffer, bytes_to_read, &bytes_read, portMAX_DELAY) == ESP_OK) {
            xQueueSend(xQueueData, in_buffer, portMAX_DELAY); // enfileirar dados lidos para a tarefa de envio
        } else {
            ESP_LOGE(I2S_TAG, "Erro durante a leitura: errno %d", errno);
            break;
        }
        vTaskDelay(1);
    }
    ESP_LOGI(I2S_TAG, "Aquisição encerrada");
    vTaskDelete(NULL);
}

static void vTaskStoreData(void * pvParameters)
{
    while (1) {
        if (xQueueReceive(xQueueData, rx_buffer, portMAX_DELAY) == pdTRUE) {
            if (audio_file) {
                fwrite(rx_buffer, 1, BUF_SIZE, audio_file);
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
    audio_file = fopen(MOUNT_POINT "/audio2.raw", "wb");

    xQueueData = xQueueCreate(DMA_BUF_NUM, BUF_SIZE*sizeof(char));
    if(xQueueData == NULL){ // testar se a criacao da fila falhou
        ESP_LOGE(MAIN_TAG, "Falha em criar fila de dados");
        while(1);
    }

    xTaskCreate(vTaskReadData,"taskREAD", 4096, NULL, 3, NULL);
    xTaskCreate(vTaskStoreData,"taskSTORE", 4096, NULL, 3, NULL); 

}