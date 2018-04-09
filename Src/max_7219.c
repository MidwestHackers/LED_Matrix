#include "max_7219.h"

enum {
    spi1 = 0x00,
    spi2 = 0x01,
    spi3 = 0x02,
};

max_7219_handle_t *max_handles[3];

void max_7219_init (max_7219_handle_t *max)
{
    assert_param(max);
    assert_param(max->hspi);
    assert_param(max->gpio_bank);

    if (max->hspi->Instance == SPI1)
    {
        max_handles[spi1] = max;
    }
    else if (max->hspi->Instance == SPI2)
    {
        max_handles[spi2] = max;
    }
    else if (max->hspi->Instance == SPI3)
    {
        max_handles[spi3] = max;
    }

    max->bus_status = bus_ready;

    // disable test mode
    max_7219_test_mode(max, 0x00);
    while (max->bus_status != bus_ready);
    // put in shutdown mode
    max_7219_display_data(max, 0x00);
    while (max->bus_status != bus_ready);
    // set scan limit to display all digits
    max_7219_write_all(max, scan_limit, 0x07);
    while (max->bus_status != bus_ready);
    // set to no decode
    max_7219_write_all(max, decode_mode, 0x00);
    while (max->bus_status != bus_ready);
}

max_7219_status_t max_7219_write_all (max_7219_handle_t *max, uint8_t reg, uint8_t data)
{
    assert_param(max);

    if (max->bus_status != bus_ready)
    {
        return status_busy;
    }

    max->bus_status = bus_busy;
    for (int i = 0; i < sizeof(max->tx_data); i+=2)
    {
        max->tx_data[i] = reg;
        max->tx_data[i+1] = data;
    }

    HAL_GPIO_WritePin(max->cs_gpio_bank, max->cs_gpio_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit_DMA(max->hspi, max->tx_data, sizeof(max->tx_data));

    return status_ok;
}

max_7219_status_t max_7219_test_mode (max_7219_handle_t *max, uint8_t enable)
{
    return max_7219_write_all(max, display_test, enable);
}

max_7219_status_t max_7219_display_data (max_7219_handle_t *max, uint8_t enable)
{
    return max_7219_write_all(max, shutdown, enable);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        HAL_GPIO_WritePin(max_handles[spi1]->cs_gpio_bank, max_handles[spi1]->cs_gpio_pin, GPIO_PIN_SET);
        max_handles[spi1]->bus_status = bus_ready;
        if (max_handles[spi1]->tx_done_handler)
        {
            (*max_handles[spi1]->tx_done_handler)();
        }
    }
    else if (hspi->Instance == SPI2)
    {
        HAL_GPIO_WritePin(max_handles[spi2]->cs_gpio_bank, max_handles[spi2]->cs_gpio_pin, GPIO_PIN_SET);
        max_handles[spi2]->bus_status = bus_ready;
        if (max_handles[spi2]->tx_done_handler)
        {
            (*max_handles[spi2]->tx_done_handler)();
        }
    }
    else if (hspi->Instance == SPI3)
    {
        HAL_GPIO_WritePin(max_handles[spi3]->cs_gpio_bank, max_handles[spi3]->cs_gpio_pin, GPIO_PIN_SET);
        max_handles[spi3]->bus_status = bus_ready;
        if (max_handles[spi3]->tx_done_handler)
        {
            (*max_handles[spi3]->tx_done_handler)();
        }
    }
}

