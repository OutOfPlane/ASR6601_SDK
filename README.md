This repo is a conglomerate of code from dragino/LA66 and asrlora/asr_lora_6601, stripped down to the bare neccessary for lorawan operation and ease of understanding.  
It is supposed to be used with Dragino LA66 Modules.  
It has a c_cpp_properties.json prepared for vscode use.  
to get started clone the repo and open the root folder in vscode.  
make sure to source env.sh and have an arm-gcc-none-eabi compiler installed and available on PATH (modifiy env.sh to add a compiler to your PATH if necessary).  
to compile the example project cd into projects/LoRaWAN_example and run make.  
To Upload run make flash



# ASR6601 SDK

ASR6601 is a general LPWAN Wireless Communication SoC, with integrated RF Transceiver, Modem and a 32-bit RISC MCU. The RF Transceiver has continuous frequency coverage from 150 MHz to 960 MHz. The Modem supports LoRa modulation for LPWAN use cases and (G)FSK modulation for legacy use cases. The Modem also supports BPSK modulation in TX and (G)MSK modulation in TX and RX. The LPWAN Wireless Communication Module designed with ASR6601 provides ultralong range and ultra-low power communication for LPWAN application.

ASR6601 can achieve a high sensitivity to -148 dBm and the maximum transmit power is up to +22 dBm. This makes the chip suitable to be used in long range LPWAN and have high efficiency. The total chip package is of very small size, QFN 6 mm x 6 mm/QFN 8 mm x 8 mm.

&nbsp;

For more details, please refer to **[ASR Documents Page](https://asriot.readthedocs.io/en/latest).**

&nbsp;

