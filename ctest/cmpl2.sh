#!/bin/bash

NAME=`basename $1 .c`
echo $NAME

riscv32-unknown-elf-as -march=rv32i -mabi=ilp32 -o start.o start.s
riscv32-unknown-elf-gcc -march=rv32i -mabi=ilp32 -u _printf_float -mstrict-align -mpreferred-stack-boundary=4 -c -o ${NAME}.o ${NAME}.c
#riscv32-unknown-elf-gcc -static-libgcc -march=rv32i -mabi=ilp32 -u _printf_float -mstrict-align -mpreferred-stack-boundary=4 -c -o ${NAME}.o ${NAME}.c

riscv32-unknown-elf-ld -b elf32-littleriscv start.o ${NAME}.o -T link.ld -o ${NAME} --no-relax /opt/riscv32i/riscv32-unknown-elf/lib/libm_nano.a /opt/riscv32i/riscv32-unknown-elf/lib/libc_nano.a /opt/riscv32i/riscv32-unknown-elf/lib/libg_nano.a /opt/riscv32i/lib/gcc/riscv32-unknown-elf/14.2.0/libgcc.a -u _printf_float --gc-sections
#riscv32-unknown-elf-ld -b elf32-littleriscv start.o ${NAME}.o -T link.ld -o ${NAME} --no-relax /opt/riscv32i.full/riscv32-unknown-elf/lib/rv32i/ilp32/libc.a /opt/riscv32i.full/riscv32-unknown-elf/lib/rv32i/ilp32/libm.a /opt/riscv32i.full/riscv32-unknown-elf/lib/rv32i/ilp32/libgloss.a /opt/riscv32i.full/lib/gcc/riscv32-unknown-elf/13.1.0/rv32i/ilp32/libgcc.a -u _printf_float --gc-sections
riscv32-unknown-elf-objdump -b elf32-littleriscv -D ${NAME} > ${NAME}.elf.dump
riscv32-unknown-elf-objcopy -O binary ${NAME} ${NAME}.bin
od -An -tx4 -w4 -v ${NAME}.bin > ${NAME}.hex

