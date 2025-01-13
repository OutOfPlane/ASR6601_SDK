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

uint8_t devEUI[] = MY_DEVEUI;
uint8_t appEUI[] = MY_APPEUI;
uint8_t appKey[] = MY_APPKEY;

TimerEvent_t deepSleepTimeout;

typedef enum
{
    STATE_JOIN,
    STATE_IDLE,
    STATE_RXTX,
    STATE_SLEEP
} deviceStates_t;


//persisitent data
deviceStates_t __attribute__(( section(".noinitdata") )) cState = STATE_JOIN;
LoRaBackupData_t __attribute__(( section(".noinitdata") )) loraSession;

void HW_GetUniqueId(uint8_t *id);
uint16_t HW_GetBatteryLevel(void);
uint16_t HW_readADC(void);

void onDeepSleepWakeup()
{
    // init_uart(9600);
    // printf("Waking from DeepSleep\r\n");
    // flushUart();
    delay_ms(20);
    NVIC_SystemReset();
}

void onLoRaJoined()
{
    printf("LoRaWAN joined\r\n");
    gpio_write(LED_RGB_PORT, LED_BLUE_PIN, GPIO_LEVEL_LOW);
    cState = STATE_IDLE;
}

void onLoRaTxRxDone(Mcps_t type, LoRaMacEventInfoStatus_t status, bool ackReceived, uint32_t channel, uint8_t dataRate, int8_t txPower, TimerTime_t txTimeOnAir)
{
    cState = STATE_SLEEP;
    gpio_write(LED_RGB_PORT, LED_GREEN_PIN, GPIO_LEVEL_LOW);
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
    init_adc();
    printf("--DONE\r\n\n");

    

    if(RCC->RST_SR & RCC_RST_SR_BOR_RESET_SR)
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
    if(RCC->RST_SR & RCC_RST_SR_CPU_RESET_SR)
    {
        printf("Software Reset detected\r\n");
        RCC->RST_SR |= RCC_RST_SR_BOR_RESET_SR;

        printf("--WARM START LORA-STACK\r\n");
        init_lora(true, true, LORA_DEV_CLASS_A);

        if(loraSession.valid)
        {
            lora_restoreSession(&loraSession);
            cState = STATE_IDLE;
        }else{
            printf("no saved session, start rejoin\r\n");
            gpio_write(LED_RGB_PORT, LED_BLUE_PIN, GPIO_LEVEL_HIGH);
            lora_join_otaa(devEUI, appEUI, appKey);
            cState = STATE_JOIN;
        }
        printf("--DONE\r\n\n");

        
    }
    

    TimerInit(&deepSleepTimeout, onDeepSleepWakeup);
    TimerSetValue(&deepSleepTimeout, 20000); // 5s limit for joining Network

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
            buflen = sprintf(buf, "Bat: %d ADC: %d", HW_GetBatteryLevel(), HW_readADC());
            if(buflen > 0)
            {
                lora_tx(buf, buflen);
                cState = STATE_RXTX;
                gpio_write(LED_RGB_PORT, LED_GREEN_PIN, GPIO_LEVEL_HIGH);
            }
            
            break;

        case STATE_SLEEP:
            TimerStart(&deepSleepTimeout);
            lora_saveSession(&loraSession);
            printf("Entering deepsleep\r\n");
            //wait for Uart to finish
            flushUart();
            RtcEnterLowPowerStopMode();
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

uint16_t readADC_mV(uint8_t adc_ch)
{
	uint16_t adc_mv = 0;

	uint16_t adc_data[10] = {0};

	float calibrated_sample[10] = {0.0};
	float vAcc = 0.0f;

    float gain_value = 1.188f;
	float dco_value = -0.107f;

    uint8_t i;

	adc_get_calibration_value(false, &gain_value, &dco_value);    

	delay_us(1000);

	adc_config_clock_division(20); // sample frequence 150K

	adc_config_sample_sequence(0, adc_ch);

	adc_config_conv_mode(ADC_CONV_MODE_CONTINUE);

	adc_enable(true);

	adc_start(true);
	for (i = 0; i < 10; i++)
	{
		while (!adc_get_interrupt_status(ADC_ISR_EOC))
			;
		adc_data[i] = adc_get_data();
	}

	adc_start(false);
	adc_enable(false);

	for (i = 0; i < 10; i++)
	{ // calibration sample value
		calibrated_sample[i] = ((1.2 / 4096) * adc_data[i] - dco_value) / gain_value;

		vAcc += calibrated_sample[i];
	}

	vAcc /= 10;

	adc_mv = vAcc * 1000;
    return adc_mv;
}


uint16_t HW_GetBatteryLevel(void)
{
	return readADC_mV(15)*3;
}

uint16_t HW_readADC()
{
    return readADC_mV(4);
}

uint16_t HW_GetTemperatureLevel(void)
{
	return 0;
}

void HW_GetUniqueId(uint8_t *id)
{
	uint32_t unique_id[2];
	system_get_chip_id(unique_id);

	id[7] = unique_id[0] >> 24 & 0xFF;
	id[6] = unique_id[0] >> 16 & 0xFF;
	id[5] = unique_id[0] >> 8 & 0xFF;
	id[4] = unique_id[0] & 0xFF;
	id[3] = unique_id[1] >> 24 & 0xFF;
	id[2] = unique_id[1] >> 16 & 0xFF;
	id[1] = unique_id[1] >> 8 & 0xFF;
	id[0] = unique_id[1] & 0xFF;
}