/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "periph.h"
#include "stm32l1xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "max_7219.h"

/* Private variables ---------------------------------------------------------*/
osThreadId ledTaskHandle;
osThreadId uart2TaskHandle;
osThreadId spiTaskHandle;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
void StartLedTask(void const * argument);
void StartUart2Task(void const * argument);
void StartSpiTask(void const * argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART2_UART_Init();
    MX_USART1_UART_Init();
    MX_SPI2_Init();
    MX_SPI3_Init();

    /* Call init function for freertos objects */
    MX_FREERTOS_Init();

    /* Start scheduler */
    osKernelStart();
    
    /* We should never get here as control is now taken by the scheduler */

    /* Infinite loop */
    while (1)
    {
    }
}

void MX_FREERTOS_Init(void) {
    /* add mutexes, ... */

    /* add semaphores, ... */

    /* start timers, add new ones, ... */

    /* add threads, ... */
    osThreadDef(ledTask, StartLedTask, osPriorityNormal, 0, 128);
    osThreadDef(uart2Task, StartUart2Task, osPriorityNormal, 0, 128);
    osThreadDef(spiTask, StartSpiTask, osPriorityNormal, 0, 128);
    ledTaskHandle = osThreadCreate(osThread(ledTask), NULL);
    uart2TaskHandle = osThreadCreate(osThread(uart2Task), NULL);
    spiTaskHandle = osThreadCreate(osThread(spiTask), NULL);

    /* add queues, ... */
}

void StartLedTask(void const * argument)
{
    for(;;)
    {
        osDelay(950);
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        osDelay(50);
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    }
}

void StartUart2Task(void const * argument)
{
    char str[] = "Hello World!";
    for(;;)
    {
        osDelay(5000);
        HAL_UART_Transmit(&huart2, (uint8_t*)&str, sizeof(str), 1000);
    }
}

volatile uint8_t hspi2_done_flag = 0;
volatile uint8_t hspi3_done_flag = 0;

void hspi2_done (void)
{
    hspi2_done_flag = 1;
}

void hspi3_done (void)
{
    hspi3_done_flag = 1;
}

void StartSpiTask(void const * argument)
{
    max_7219_handle_t upper = {
        .hspi = &hspi2,
        .cs_gpio_bank = GPIOB,
        .cs_gpio_pin = GPIO_PIN_12,
        .tx_done_handler = &hspi2_done,
    };
    max_7219_handle_t lower = {
        .hspi = &hspi3,
        .cs_gpio_bank = GPIOC,
        .cs_gpio_pin = GPIO_PIN_11,
        .tx_done_handler = &hspi3_done,
    };
    max_7219_init(&upper);
    max_7219_init(&lower);

    // take the modules in and out of test mode as long as the user button is held down
    for(;;)
    {
        // User button is active low
        while (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET)
        {
            osDelay(10);
        }

        /*
        max_7219_test_mode(&upper, 1);
        max_7219_test_mode(&lower, 1);
        osDelay(500);
        max_7219_test_mode(&upper, 0);
        max_7219_test_mode(&lower, 0);
        osDelay(500);
        */

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_0, 0x01);
        max_7219_write_all(&lower, digit_0, 0x01);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_1, 0x02);
        max_7219_write_all(&lower, digit_1, 0x02);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_2, 0x04);
        max_7219_write_all(&lower, digit_2, 0x04);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_3, 0x08);
        max_7219_write_all(&lower, digit_3, 0x08);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_4, 0x10);
        max_7219_write_all(&lower, digit_4, 0x10);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_5, 0x20);
        max_7219_write_all(&lower, digit_5, 0x20);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_6, 0x40);
        max_7219_write_all(&lower, digit_6, 0x40);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_7, 0x80);
        max_7219_write_all(&lower, digit_7, 0x80);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);
        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_display_data(&upper, 0x01);
        max_7219_display_data(&lower, 0x01);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        osDelay(500);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_0, 0x00);
        max_7219_write_all(&lower, digit_0, 0x00);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_1, 0x00);
        max_7219_write_all(&lower, digit_1, 0x00);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_2, 0x00);
        max_7219_write_all(&lower, digit_2, 0x00);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_3, 0x00);
        max_7219_write_all(&lower, digit_3, 0x00);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_4, 0x00);
        max_7219_write_all(&lower, digit_4, 0x00);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_5, 0x00);
        max_7219_write_all(&lower, digit_5, 0x00);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_6, 0x00);
        max_7219_write_all(&lower, digit_6, 0x00);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_write_all(&upper, digit_7, 0x00);
        max_7219_write_all(&lower, digit_7, 0x00);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        hspi2_done_flag = 0;
        hspi3_done_flag = 0;
        max_7219_display_data(&upper, 0x01);
        max_7219_display_data(&lower, 0x01);
        while (!hspi2_done_flag);
        while (!hspi3_done_flag);

        osDelay(500);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /* Configure the main internal regulator output voltage */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    /*Initializes the CPU, AHB and APB busses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /* Configure the Systick interrupt time */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /* Configure the Systick */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        HAL_IncTick();
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
    while(1)
    {
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
