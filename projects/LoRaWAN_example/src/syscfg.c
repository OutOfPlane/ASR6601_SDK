#include "tremo_uart.h"
#include "tremo_i2c.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_rtc.h"
#include "lora_config.h"
#include "syscfg.h"

void init_uart(uint32_t baudrate)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);

    uart_config_t uart_config;

    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1); // UART0_TX:GP17

    // uart init
    uart_config_init(&uart_config);
    uart_config.fifo_mode = ENABLE;
    uart_config.mode = UART_MODE_TX;
    uart_config.baudrate = baudrate;
    uart_init(UART0, &uart_config);

    uart_cmd(UART0, ENABLE);
}

void init_gpio()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    gpio_init(LED_RGB_PORT, LED_RED_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(LED_RGB_PORT, LED_GREEN_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(LED_RGB_PORT, LED_BLUE_PIN, GPIO_MODE_OUTPUT_PP_LOW);
    gpio_init(BOOST_EN_PORT, BOOST_EN_PIN, GPIO_MODE_OUTPUT_PP_LOW);
}

void init_rtc()
{
    //this is important for wakeup to work
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);

    rcc_set_rtc_clk_source(RCC_RTC_CLK_SOURCE_XO32K);
    rcc_enable_oscillator(RCC_OSC_XO32K, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    rtc_calendar_cmd(ENABLE);
    NVIC_EnableIRQ(RTC_IRQn);
}

void flushUart()
{
    while (!(UART0->FR & UART_FLAG_TX_FIFO_EMPTY));    
}

void init_i2c()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_I2C0, true);

    gpio_set_iomux(GPIOA, GPIO_PIN_14, 3);
    gpio_set_iomux(GPIOA, GPIO_PIN_15, 3);

    i2c_config_t cfg;
    i2c_config_init(&cfg);
    cfg.fifo_mode_en = false;
    cfg.mode = I2C_MODE_MASTER;
    cfg.settings.master.speed = I2C_SPEED_FAST;
    i2c_init(I2C0, &cfg);
    i2c_cmd(I2C0, ENABLE);
}