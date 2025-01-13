#ifndef MY_LORA_H
#define MY_LORA_H
#include "stdint.h"
#include "stdbool.h"

typedef enum {
    /*!
     * LoRaWAN device class A
     *
     * LoRaWAN Specification V1.0.2, chapter 3
     */
    LORA_DEV_CLASS_A,
    /*!
     * LoRaWAN device class B
     *
     * LoRaWAN Specification V1.0.2, chapter 8
     */
    LORA_DEV_CLASS_B,
    /*!
     * LoRaWAN device class C
     *
     * LoRaWAN Specification V1.0.2, chapter 17
     */
    LORA_DEV_CLASS_C,
} LoraDeviceClass_t;

typedef struct{
    bool valid;
    uint8_t nkwSKey[16];
    uint8_t appSKey[16];
    uint32_t netID;
    uint32_t devAddr;
    uint32_t uplinkCounter;
    uint32_t downlinkCounter;
} LoRaBackupData_t;

void init_lora(bool ADREnable, bool publicNetwork, LoraDeviceClass_t deviceClass);
bool lora_join_otaa(uint8_t *deviceEUI, uint8_t *appEUI, uint8_t *appKey);
void lora_tx(uint8_t *buffer, uint8_t len);
void lora_saveSession(LoRaBackupData_t *data);
void lora_restoreSession(LoRaBackupData_t *data);

#endif