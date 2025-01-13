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
extern void onLoRaJoined();
extern void onLoRaTxRxDone(Mcps_t type, LoRaMacEventInfoStatus_t status, bool ackReceived, uint32_t channel, uint8_t dataRate, int8_t txPower, TimerTime_t txTimeOnAir);

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
    MlmeReq_t req;
    req.Type = MLME_JOIN;
    req.Req.Join.DevEui = deviceEUI;
    req.Req.Join.AppEui = appEUI;
    req.Req.Join.AppKey = appKey;
    req.Req.Join.NbTrials = 8;

    if (LoRaMacMlmeRequest(&req) == LORAMAC_STATUS_OK)
    {
        return true;
    }
    else
    {
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

void lora_saveSession(LoRaBackupData_t *data)
{
    data->valid = true;

    MibRequestConfirm_t mib;
    mib.Type = MIB_NWK_SKEY;
    mib.Param.NwkSKey = NULL;

    if (LoRaMacMibGetRequestConfirm(&mib) == LORAMAC_STATUS_OK)
    {
        for (size_t i = 0; i < 16; i++)
        {
            data->nkwSKey[i] = mib.Param.NwkSKey[i];
        }
    }else{
        data->valid = false;
        printf("unable to retrieve network SKey\r\n");
    }

    mib.Type = MIB_APP_SKEY;
    mib.Param.AppSKey = NULL;
    if (LoRaMacMibGetRequestConfirm(&mib) == LORAMAC_STATUS_OK)
    {
        for (size_t i = 0; i < 16; i++)
        {
            data->appSKey[i] = mib.Param.AppSKey[i];
        }
    }else{
        data->valid = false;
        printf("unable to retrieve app SKey\r\n");
    }

    mib.Type = MIB_NET_ID;
    if (LoRaMacMibGetRequestConfirm(&mib) == LORAMAC_STATUS_OK)
    {
        data->netID = mib.Param.NetID;
    }else{
        data->valid = false;
        printf("unable to retrieve net id\r\n");
    }

    mib.Type = MIB_DEV_ADDR;
    if (LoRaMacMibGetRequestConfirm(&mib) == LORAMAC_STATUS_OK)
    {
        data->devAddr = mib.Param.DevAddr;
    }else{
        data->valid = false;
        printf("unable to retrieve dev Addr\r\n");
    }

    mib.Type = MIB_UPLINK_COUNTER;
    if (LoRaMacMibGetRequestConfirm(&mib) == LORAMAC_STATUS_OK)
    {
        data->uplinkCounter = mib.Param.UpLinkCounter;
    }else{
        data->valid = false;
        printf("unable to retrieve uplink ctr\r\n");
    }

    mib.Type = MIB_DOWNLINK_COUNTER;
    if (LoRaMacMibGetRequestConfirm(&mib) == LORAMAC_STATUS_OK)
    {
        data->downlinkCounter = mib.Param.DownLinkCounter;
    }else{
        data->valid = false;
        printf("unable to retrieve downlink ctr\r\n");
    }
}

void lora_restoreSession(LoRaBackupData_t *data)
{
    MibRequestConfirm_t mib;
    mib.Type = MIB_NWK_SKEY;
    mib.Param.NwkSKey = data->nkwSKey;
    LoRaMacMibSetRequestConfirm(&mib);

    mib.Type = MIB_APP_SKEY;
    mib.Param.AppSKey = data->appSKey;
    LoRaMacMibSetRequestConfirm(&mib);

    mib.Type = MIB_NET_ID;
    mib.Param.NetID = data->netID;
    LoRaMacMibSetRequestConfirm(&mib);

    mib.Type = MIB_DEV_ADDR;
    mib.Param.DevAddr = data->devAddr;
    LoRaMacMibSetRequestConfirm(&mib);

    mib.Type = MIB_UPLINK_COUNTER;
    mib.Param.UpLinkCounter = data->uplinkCounter;
    LoRaMacMibSetRequestConfirm(&mib);

    mib.Type = MIB_DOWNLINK_COUNTER;
    mib.Param.DownLinkCounter = data->downlinkCounter;
    LoRaMacMibSetRequestConfirm(&mib);

    mib.Type = MIB_NETWORK_JOINED;
    mib.Param.IsNetworkJoined = true;
    LoRaMacMibSetRequestConfirm(&mib);
}

void McpsConfirm(McpsConfirm_t *data)
{
    onLoRaTxRxDone(data->McpsRequest, data->Status, data->AckReceived, data->Channel, data->Datarate, data->TxPower, data->TxTimeOnAir);
}

void McpsIndication(McpsIndication_t *data)
{
    printf("McpsIndication type:%d\r\n", data->McpsIndication);
}

void MlmeConfirm(MlmeConfirm_t *data)
{

    switch (data->MlmeRequest)
    {
    case MLME_JOIN:
    {
        if (data->Status == LORAMAC_EVENT_INFO_STATUS_OK)
        {
            printf("JOIN Successful\r\n");
            onLoRaJoined();
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
        printf("## MlmeConfirm - UNHANDELED MLME request type\r\n");
        break;
    }
}

void MlmeIndication(MlmeIndication_t *data)
{
    printf("MlmeIndication type:%d\r\n", data->MlmeIndication);
}

uint8_t getBatteryLevel()
{
    return 0;
}

float getTemperature()
{
    return 0.0f;
}
