# Lab 3: STM32F429I-DISC1 code relocation to external SDRAM

## Build

```sh
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi gdb-multiarch openocd make
make
arm-none-eabi-objdump -h firmware.elf
arm-none-eabi-nm -n firmware.elf | grep -E 'sdram_add|__sdram_text|_sidata'
make flash
```

## Debug

Start `openocd -f interface/stlink.cfg -f target/stm32f4x.cfg` in one terminal.
In another: `gdb-multiarch firmware.elf`, then:

```gdb
target extended-remote localhost:3333
monitor reset halt
break sdram_add
continue
info registers pc
x/8i $pc
finish
p/x lab3_pc
p/x lab3_result
p lab3_calls
x/8wx 0xD0000000
p/x fault_cfsr
```

Expected `lab3_pc` lies in 0xD0000000..0xD07FFFFF, `lab3_calls` becomes 1, green LED PG13 lights, and `lab3_result` matches `(0x12345678 ^ 0x13579BDF) + 0x11111111` modulo 2^32.

## Notes

- SDRAM pin table and FMC bank-2 settings follow the supplied Lab 2 approach; timings remain provisional pending fitted SDRAM speed-grade verification.
- Uses internal 16 MHz HSI for PLL (168 MHz CPU, 84 MHz SDRAM) rather than assuming board HSE configuration.
- MPU region 0: executable, Normal, non-cacheable 8 MiB SDRAM. Default map is retained for other privileged accesses.
- SDRAM relocation runs only after FMC initialization. No initialized `.data` or stack is placed in SDRAM.
- The demonstration relocated function is intentionally self-contained. For larger relocated code, inspect all branch and literal references and relocate dependencies together.
- Test firmware in a debugger first. A failed FMC access or MPU configuration may trigger MemManage/BusFault/HardFault.
