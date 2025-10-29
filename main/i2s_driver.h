#pragma once

#define DMA_BUF_NUM 16 // quantidade de buffers do DMA
#define DMA_BUF_SIZE 1024 // tamanho [em amostras] dos buffers do DMA
#define BUF_SIZE 4096 // tamanho [em bytes] dos buffers de entrada/transmissao

void i2s_pdm_init();
