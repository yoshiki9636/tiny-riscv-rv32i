riscv32-unknown-elf-objdump -b elf32-littleriscv -D build/RTOSDemo32.axf > build/RTOSDemo32.elf.dump
riscv32-unknown-elf-objcopy -O binary build/RTOSDemo32.axf build/RTOSDemo32.bin
od -An -tx4 -w4 -v build/RTOSDemo32.bin > build/RTOSDemo32.hex
cp build/RTOSDemo32.hex /mnt/d/riscv/tiny-riscv-rv32i/ctest
 
