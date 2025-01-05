arm-none-eabi-objdump -d ./out/lora_firmware.elf > ./out/objdump.lst
arm-none-eabi-nm ./out/lora_firmware.elf | sort -k1 > ./out/vars.lst