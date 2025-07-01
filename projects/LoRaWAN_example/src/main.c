#include <stdio.h>
#include "tremo_delay.h"
#include "radio.h"
#include "lora_config.h"
#include "stdio.h"
#include "syscfg.h"
#include "lora.h"
#include "no_git_add_access.h" //comment this out when you run it and replace MY_DEVEUI & others with your WAN access tokens
#include "timer.h"
#include "LoRaMac.h"
#include "rtc-board.h"
#include "tremo_adc.h"
#include "tremo_i2c.h"
#include "ads1115.h"

uint8_t devEUI[] = MY_DEVEUI;
uint8_t appEUI[] = MY_APPEUI;
uint8_t appKey[] = MY_APPKEY;

TimerEvent_t deepSleepTimeout;

typedef enum
{
    STATE_JOIN,
    STATE_IDLE,
    STATE_RXTX,
    STATE_SLEEP,
    STATE_WAIT
} deviceStates_t;

// persisitent data
deviceStates_t __attribute__((section(".noinitdata"))) cState = STATE_JOIN;
LoRaBackupData_t __attribute__((section(".noinitdata"))) loraSession;

// void HW_GetUniqueId(uint8_t *id);
uint16_t HW_GetBatteryLevel(void);
uint16_t HW_readADC(void);

void onDeepSleepWakeup()
{
    //delay is important for lora HW to work after reset
    delay_ms(20);
    NVIC_SystemReset();
}

void onLoRaJoined()
{
    printf("LoRaWAN joined\r\n");
    gpio_write(LED_RGB_PORT, LED_BLUE_PIN, GPIO_LEVEL_LOW);
    cState = STATE_IDLE;
}

void onLoRaJoinError()
{
    printf("LoRaWAN join failed\r\n");
    gpio_write(LED_RGB_PORT, LED_BLUE_PIN, GPIO_LEVEL_LOW);
    gpio_write(LED_RGB_PORT, LED_RED_PIN, GPIO_LEVEL_HIGH);
    delay_ms(1000);
    gpio_write(LED_RGB_PORT, LED_RED_PIN, GPIO_LEVEL_LOW);
    cState = STATE_SLEEP;
}

void onLoRaTxRxDone(Mcps_t type, LoRaMacEventInfoStatus_t status, bool ackReceived, uint32_t channel, uint8_t dataRate, int8_t txPower, TimerTime_t txTimeOnAir)
{
    cState = STATE_SLEEP;
    gpio_write(LED_RGB_PORT, LED_GREEN_PIN, GPIO_LEVEL_LOW);
    if(status != LORAMAC_EVENT_INFO_STATUS_OK) {
        gpio_write(LED_RGB_PORT, LED_RED_PIN, GPIO_LEVEL_HIGH);
        delay_ms(1000);
        gpio_write(LED_RGB_PORT, LED_RED_PIN, GPIO_LEVEL_LOW);
    }else{
        lora_saveSession(&loraSession);
    }
    printf("RXTX Done\r\n");
    printf(" MCPS: %d\r\n STATUS: %d\r\n ACK: %d\r\n CH: %ld\r\n DR: %d\r\n TXPOW: %d\r\n TxTOA: %ld\r\n\n", type, status, ackReceived, channel, dataRate, txPower, (uint32_t)txTimeOnAir);
}

int main(void)
{
    init_uart(9600); // uart is configured tx only
    printf("Starting LoRaWAN Example\r\n");
    printf("--INITIALIZING HARDWARE\r\n");
    init_gpio();
    init_rtc();
    init_i2c();
    // wait for 5V to come up and sensors to stabalize
    delay_ms(500);
    
    printf("--DONE\r\n\n");

    gpio_config_stop3_wakeup(BUTTON_PORT, BUTTON_PIN, true, GPIO_LEVEL_LOW);

    uint8_t ch_data[8];
    uint32_t rx;
    for (size_t i = 0; i < 4; i++)
    {
        rx = ads1115_readCh(i);
        rx = rx*1875/10000;
        ch_data[2*i] = rx & 0xFF;
        ch_data[2*i+1] = (rx>>8) & 0xFF;
    }
    



    if (RCC->RST_SR & RCC_RST_SR_BOR_RESET_SR)
    {
        printf("Hard Reset detected\r\n");
        RCC->RST_SR |= RCC_RST_SR_BOR_RESET_SR;

        printf("--COLD START LORA-STACK\r\n");
        init_lora(true, true, LORA_DEV_CLASS_A);
        loraSession.valid = false;
        printf("--DONE\r\n\n");

        printf("--CONNECTING LORA NETWORK\r\n");
        gpio_write(LED_RGB_PORT, LED_BLUE_PIN, GPIO_LEVEL_HIGH);
        lora_join_otaa(devEUI, appEUI, appKey);
        cState = STATE_JOIN;
    }
    if (RCC->RST_SR & RCC_RST_SR_CPU_RESET_SR)
    {
        printf("Software Reset detected\r\n");
        RCC->RST_SR |= RCC_RST_SR_BOR_RESET_SR;

        printf("--WARM START LORA-STACK\r\n");
        init_lora(true, true, LORA_DEV_CLASS_A);

        if (loraSession.valid)
        {
            lora_restoreSession(&loraSession);
            cState = STATE_IDLE;
        }
        else
        {
            printf("no saved session, start rejoin\r\n");
            gpio_write(LED_RGB_PORT, LED_BLUE_PIN, GPIO_LEVEL_HIGH);
            lora_join_otaa(devEUI, appEUI, appKey);
            cState = STATE_JOIN;
        }
        printf("--DONE\r\n\n");
    }

    TimerInit(&deepSleepTimeout, onDeepSleepWakeup);
    TimerSetValue(&deepSleepTimeout, 600000); // 10min interval for measurements

    char buf[30];
    int buflen;

    /* Infinite loop */
    while (1)
    {
        switch (cState)
        {
        case STATE_JOIN:
        case STATE_RXTX:
            /* code */
            break;
        case STATE_IDLE:
            printf("Sending LoRa MSG\r\n");

            lora_tx(ch_data, sizeof(ch_data));
            cState = STATE_RXTX;
            gpio_write(LED_RGB_PORT, LED_GREEN_PIN, GPIO_LEVEL_HIGH);

            break;

        case STATE_SLEEP:
            TimerStart(&deepSleepTimeout);
            printf("Entering deepsleep\r\n");
            gpio_write(BOOST_EN_PORT, BOOST_EN_PIN, GPIO_LEVEL_HIGH);
            // wait for Uart to finish
            flushUart();
            RtcEnterLowPowerStopMode();
            onDeepSleepWakeup();
            break;

        default:
            break;
        }

        // this is neccessary, so the radio handles data and interrupts
        if (Radio.IrqProcess != NULL)
        {
            Radio.IrqProcess();
        }
    }
}

uint16_t HW_GetBatteryLevel(void)
{
    return 100;
}

uint16_t HW_readADC()
{
    return 50;
}

uint16_t HW_GetTemperatureLevel(void)
{
    return 0;
}

// void HW_GetUniqueId(uint8_t *id)
// {
// 	uint32_t unique_id[2];
// 	system_get_chip_id(unique_id);

// 	id[7] = unique_id[0] >> 24 & 0xFF;
// 	id[6] = unique_id[0] >> 16 & 0xFF;
// 	id[5] = unique_id[0] >> 8 & 0xFF;
// 	id[4] = unique_id[0] & 0xFF;
// 	id[3] = unique_id[1] >> 24 & 0xFF;
// 	id[2] = unique_id[1] >> 16 & 0xFF;
// 	id[1] = unique_id[1] >> 8 & 0xFF;
// 	id[0] = unique_id[1] & 0xFF;
// }