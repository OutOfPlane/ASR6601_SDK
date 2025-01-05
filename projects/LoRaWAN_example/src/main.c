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

uint8_t devEUI[] = MY_DEVEUI;
uint8_t appEUI[] = MY_APPEUI;
uint8_t appKey[] = MY_APPKEY;

TimerEvent_t stateTimeout;

typedef enum
{
    STATE_JOIN,
    STATE_IDLE,
    STATE_RXTX,
    STATE_SLEEP
} deviceStates_t;

deviceStates_t cState = STATE_JOIN;

void onStateTimeout()
{
    printf("Timeout waiting for State Switch\r\n");
}

void onLoRaJoined()
{
    printf("LoRaWAN joined\r\n");
    cState = STATE_IDLE;
}

void onLoRaTxRxDone(Mcps_t type, LoRaMacEventInfoStatus_t status, bool ackReceived, uint32_t channel, uint8_t dataRate, int8_t txPower, TimerTime_t txTimeOnAir)
{
    cState = STATE_IDLE;
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
    printf("--DONE\r\n\n");

    printf("--INITIALIZING LORA-STACK\r\n");
    init_lora(true, true, LORA_DEV_CLASS_A);
    printf("--DONE\r\n\n");

    printf("--CONNECTING LORA NETWORK\r\n");
    lora_join_otaa(devEUI, appEUI, appKey);

    TimerInit(&stateTimeout, onStateTimeout);
    TimerSetValue(&stateTimeout, 5000); // 5s limit for joining Network

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
            printf("Sending Hello MSG\r\n");
            lora_tx("Hello World", sizeof("Hello World"));
            cState = STATE_RXTX;
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
