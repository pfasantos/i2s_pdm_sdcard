#include"i2s_driver.h"
#include "esp_err.h"
#include "esp_log.h"

#include "driver/i2s_pdm.h"
#include "driver/gpio.h"


char in_buffer[BUF_SIZE]; // buffer para entrada de dados

i2s_chan_handle_t rx_handle; // handler para canais i2s

// tags
static const char *I2S_TAG = "I2S";

void i2s_pdm_init()
{
    // configuracao do canal i2s
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0, // escolha do canal i2s 0 (no esp32 apenas esse canal suporta o modo pdm)
        .role = I2S_ROLE_MASTER, // o esp32 atua como o controlador
        .dma_desc_num = DMA_BUF_NUM, // quantidade de buffers do DMA
        .dma_frame_num = DMA_BUF_SIZE, // tamanho dos buffers do DMA
        .auto_clear = false, // limpar automaticamente o buffer TX (desnecessario)
    };

    // alocar o canal i2s para receber dados
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);

    // configuracoes de clock, slot e gpio para o i2s no modo pdm
    i2s_pdm_rx_config_t pdm_rx_cfg = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(44100), // 44100 amostras por segundo
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO), // 16bits por amostras e audio mono
        .gpio_cfg = {
            .clk = GPIO_NUM_18, // clock
            .din = GPIO_NUM_19, // data in
            .invert_flags = { // nao inverter bits
                .clk_inv = false,
            },
        },
    };

    // inicializar o canal i2s no modo pdm
    i2s_channel_init_pdm_rx_mode(rx_handle, &pdm_rx_cfg);
    ESP_LOGI(I2S_TAG, "Canal I2S iniciado.");
}