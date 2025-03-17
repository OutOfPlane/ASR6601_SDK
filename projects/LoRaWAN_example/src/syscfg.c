#include "tremo_uart.h"
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
    gpio_init(BOOST_EN_PORT, BOOST_EN_PIN, GPIO_MODE_OUTPUT_PP_HIGH);
}

void init_rtc()
{
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