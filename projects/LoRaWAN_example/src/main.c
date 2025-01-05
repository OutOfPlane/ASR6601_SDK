#include <stdio.h>
#include "tremo_delay.h"
#include "radio.h"
#include "lora_config.h"
#include "stdio.h"
#include "syscfg.h"
#include "lora.h"
#include "no_git_add_access.h" //comment this out when you run it and replace MY_DEVEUI & others with your WAN access tokens


uint8_t devEUI[] = MY_DEVEUI;
uint8_t appEUI[] = MY_APPEUI;
uint8_t appKey[] = MY_APPKEY;

void onLoraJoined()
{
    printf("Sending Hello MSG\r\n");
    lora_tx("Hello World", sizeof("Hello World"));
}

int main(void)
{
    init_uart(9600); //uart is configured tx only
    printf("Starting VV-LORA-FW\r\n");
    printf("--INITIALIZING HARDWARE\r\n");
    init_gpio();
    init_rtc();
    printf("--DONE\r\n\n");
    
    
    printf("--INITIALIZING LORA-STACK\r\n");
    init_lora(true, true, LORA_DEV_CLASS_A);
    printf("--DONE\r\n\n");

    printf("--CONNECTING LORA NETWORK\r\n");
    lora_join_otaa(devEUI, appEUI, appKey);

    /* Infinite loop */
    while (1)
    {
        //this is neccessary, so the radio handles data and interrupts
        if (Radio.IrqProcess != NULL)
		{
			Radio.IrqProcess();
		}
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(void *file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1)
    {
    }
}
#endif
