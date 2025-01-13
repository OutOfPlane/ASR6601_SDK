arm-none-eabi-objdump -d ./out/LoRaWAN_example.elf > ./out/objdump.lst
arm-none-eabi-nm ./out/LoRaWAN_example.elf | sort -k1 > ./out/vars.lst