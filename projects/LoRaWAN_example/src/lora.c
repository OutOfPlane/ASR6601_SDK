#include "lora.h"
#include "LoRaMac.h"
#include "stdio.h"

void McpsConfirm(McpsConfirm_t *data);
void McpsIndication(McpsIndication_t *data);
void MlmeConfirm(MlmeConfirm_t *data);
void MlmeIndication(MlmeIndication_t *data);

uint8_t getBatteryLevel();
float getTemperature();

extern void LoRaMacTestRxWindowsOn(bool enable);
extern void onLoraJoined();

// why??
uint8_t decrypt_flag = 0;

LoRaMacPrimitives_t loraPrimitives;
LoRaMacCallback_t loraCallbacks;

void init_lora(bool ADREnable, bool publicNetwork, LoraDeviceClass_t deviceClass)
{
    printf("init_lora\r\n");

    loraPrimitives.MacMcpsConfirm = McpsConfirm;
    loraPrimitives.MacMcpsIndication = McpsIndication;
    loraPrimitives.MacMlmeConfirm = MlmeConfirm;
    loraPrimitives.MacMlmeIndication = MlmeIndication;

    loraCallbacks.GetBatteryLevel = getBatteryLevel;
    loraCallbacks.GetTemperatureLevel = getTemperature;

    printf("lora mac init\r\n");
    LoRaMacInitialization(&loraPrimitives, &loraCallbacks, LORAMAC_REGION_EU868);

    printf("set adr\r\n");
    MibRequestConfirm_t mib;
    mib.Type = MIB_ADR;
    mib.Param.AdrEnable = ADREnable;
    LoRaMacMibSetRequestConfirm(&mib);

    printf("set network type\r\n");
    mib.Type = MIB_PUBLIC_NETWORK;
    mib.Param.EnablePublicNetwork = publicNetwork;
    LoRaMacMibSetRequestConfirm(&mib);

    printf("set dev class\r\n");
    mib.Type = MIB_DEVICE_CLASS;
    if (deviceClass == LORA_DEV_CLASS_A)
        mib.Param.Class = CLASS_A;
    if (deviceClass == LORA_DEV_CLASS_B)
        mib.Param.Class = CLASS_B;
    if (deviceClass == LORA_DEV_CLASS_C)
        mib.Param.Class = CLASS_C;
    LoRaMacMibSetRequestConfirm(&mib);
}

bool lora_join_otaa(uint8_t *deviceEUI, uint8_t *appEUI, uint8_t *appKey)
{
    printf("lora_join_otaa\r\n");
    MlmeReq_t req;
    req.Type = MLME_JOIN;
    req.Req.Join.DevEui = deviceEUI;
    req.Req.Join.AppEui = appEUI;
    req.Req.Join.AppKey = appKey;
    req.Req.Join.NbTrials = 8;

    if (LoRaMacMlmeRequest(&req) == LORAMAC_STATUS_OK)
    {
        printf("Request Successful\r\n");
        return true;
    }
    else
    {
        printf("Request Failed\r\n");
        return false;
    }
}

void lora_tx(uint8_t *buffer, uint8_t len)
{
    McpsReq_t req;
    LoRaMacTxInfo_t txinfo;
    if (LoRaMacQueryTxPossible(len, &txinfo) != LORAMAC_STATUS_OK)
    {
        // Send empty frame in order to flush MAC commands
		req.Type = MCPS_UNCONFIRMED;
		req.Req.Unconfirmed.fBuffer = NULL;
		req.Req.Unconfirmed.fBufferSize = 0;
		req.Req.Unconfirmed.Datarate = 0;
    }
    else
    {
        req.Type = MCPS_UNCONFIRMED;
        req.Req.Unconfirmed.fPort = 1;
        req.Req.Unconfirmed.fBufferSize = len;
        req.Req.Unconfirmed.fBuffer = buffer;
        req.Req.Unconfirmed.Datarate = 0;
    }
    LoRaMacMcpsRequest(&req);
}

void McpsConfirm(McpsConfirm_t *data)
{
    printf("McpsConfirm\r\n");
}

void McpsIndication(McpsIndication_t *data)
{
    printf("McpsIndication\r\n");
}

void MlmeConfirm(MlmeConfirm_t *data)
{
    printf("MlmeConfirm\r\n");

    switch (data->MlmeRequest)
    {
    case MLME_JOIN:
    {
        if (data->Status == LORAMAC_EVENT_INFO_STATUS_OK)
        {
            printf("JOIN Successful\r\n");
            onLoraJoined();
        }
        else
        {
            printf("JOIN Failed, Status: %d\r\n", data->Status);
        }
        break;
    }
    case MLME_LINK_CHECK:
    {
        if (data->Status == LORAMAC_EVENT_INFO_STATUS_OK)
        {
            printf("LINK CHECK Successful\r\n");
        }
        else
        {
            printf("LINK CHECK Failed, Status: %d\r\n", data->Status);
        }
        break;
    }
    default:
        break;
    }
}

void MlmeIndication(MlmeIndication_t *data)
{
    printf("MlmeIndication\r\n");
}

uint8_t getBatteryLevel()
{
    return 0;
}

float getTemperature()
{
    return 0.0f;
}
