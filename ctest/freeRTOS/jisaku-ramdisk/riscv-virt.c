/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include <FreeRTOS.h>

#include <string.h>

#include "riscv-virt.h"
#include "htif.h"

int xGetCoreID( void )
{
	int id;

	__asm ("csrr %0, mhartid" : "=r" ( id ) );

	return id;
}

void uprint( const char* buf, int length, int ret ) {
    //unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* uart_out = (unsigned int*)0xc000fc00;
    unsigned int* uart_status = (unsigned int*)0xc000fc04;

    for (int i = 0; i < length + ret; i++) {
        unsigned int flg = 1;
        while(flg == 1) {
            flg = *uart_status;
        }
        *uart_out = ((i == length+1)&&(ret == 2)) ? 0x0a :
                    ((i == length)&&(ret == 1)) ? 0x20 :
                    ((i == length)&&(ret == 2)) ? 0x0d : buf[i];
        //*led = i;
    }
}

/* Use a debugger to set this to 0 if this binary was loaded through gdb instead
 * of spike's ELF loader. HTIF only works if spike's ELF loader was used. */
volatile int use_htif = 1;

void vSendString( const char *s )
{
	int len = strlen(s);
	uprint( s, len, 0);
/*
	portENTER_CRITICAL();

	if (use_htif) {
		while (*s) {
			htif_putc(*s);
			s++;
		}
		htif_putc('\n');
	}

	portEXIT_CRITICAL();
*/
}

void handle_trap(void)
{
	while (1)
		;
}
