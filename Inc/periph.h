#ifndef __periph_h__
#define __periph_h__
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l1xx_hal.h"

/* Public variables ----------------------------------------------------------*/
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* Public function prototypes ------------------------------------------------*/
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_SPI2_Init(void);
void MX_SPI3_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__periph_h__ */
