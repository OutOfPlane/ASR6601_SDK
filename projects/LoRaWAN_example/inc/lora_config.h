#ifndef __LORA_CONFIG_H
#define __LORA_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tremo_gpio.h"

#define CONFIG_LORA_RFSW_CTRL_GPIOX GPIOD
#define CONFIG_LORA_RFSW_CTRL_PIN   GPIO_PIN_11

#define CONFIG_LORA_RFSW_VDD_GPIOX GPIOA
#define CONFIG_LORA_RFSW_VDD_PIN   GPIO_PIN_10


#if defined( DRAGINO_LA66 )	
#define LED_RGB_PORT   GPIOB
#define LED_RED_PIN    GPIO_PIN_13
#define LED_GREEN_PIN  GPIO_PIN_14
#define LED_BLUE_PIN   GPIO_PIN_12

#define BOOST_EN_PORT   GPIOB
#define BOOST_EN_PIN   GPIO_PIN_7
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LORA_CONFIG_H */
