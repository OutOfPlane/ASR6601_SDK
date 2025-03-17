#ifndef SYSCFG_H
#define SYSCFG_H
#include "stdint.h"

void init_uart(uint32_t baudrate);
void init_gpio();
void init_rtc();
void flushUart();


#endif