/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
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

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the RISC-V port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"

/* Standard includes. */
#include "string.h"
#include "riscv-virt.h"

#ifdef configCLINT_BASE_ADDRESS
    #warning "The configCLINT_BASE_ADDRESS constant has been deprecated. configMTIME_BASE_ADDRESS and configMTIMECMP_BASE_ADDRESS are currently being derived from the (possibly 0) configCLINT_BASE_ADDRESS setting.  Please update to define configMTIME_BASE_ADDRESS and configMTIMECMP_BASE_ADDRESS directly in place of configCLINT_BASE_ADDRESS. See www.FreeRTOS.org/Using-FreeRTOS-on-RISC-V.html"
#endif

#ifndef configMTIME_BASE_ADDRESS
    #warning "configMTIME_BASE_ADDRESS must be defined in FreeRTOSConfig.h. If the target chip includes a memory-mapped mtime register then set configMTIME_BASE_ADDRESS to the mapped address.  Otherwise set configMTIME_BASE_ADDRESS to 0.  See www.FreeRTOS.org/Using-FreeRTOS-on-RISC-V.html"
#endif

#ifndef configMTIMECMP_BASE_ADDRESS
    #warning "configMTIMECMP_BASE_ADDRESS must be defined in FreeRTOSConfig.h. If the target chip includes a memory-mapped mtimecmp register then set configMTIMECMP_BASE_ADDRESS to the mapped address.  Otherwise set configMTIMECMP_BASE_ADDRESS to 0.  See www.FreeRTOS.org/Using-FreeRTOS-on-RISC-V.html"
#endif

/* Let the user override the pre-loading of the initial RA. */
#ifdef configTASK_RETURN_ADDRESS
    #define portTASK_RETURN_ADDRESS    configTASK_RETURN_ADDRESS
#else
    #define portTASK_RETURN_ADDRESS    0
#endif

/* The stack used by interrupt service routines.  Set configISR_STACK_SIZE_WORDS
 * to use a statically allocated array as the interrupt stack.  Alternative leave
 * configISR_STACK_SIZE_WORDS undefined and update the linker script so that a
 * linker variable names __freertos_irq_stack_top has the same value as the top
 * of the stack used by main.  Using the linker script method will repurpose the
 * stack that was used by main before the scheduler was started for use as the
 * interrupt stack after the scheduler has started. */
#ifdef configISR_STACK_SIZE_WORDS
static __attribute__( ( aligned( 16 ) ) ) StackType_t xISRStack[ configISR_STACK_SIZE_WORDS ] = { 0 };
const StackType_t xISRStackTop = ( StackType_t ) &( xISRStack[ configISR_STACK_SIZE_WORDS & ~portBYTE_ALIGNMENT_MASK ] );

/* Don't use 0xa5 as the stack fill bytes as that is used by the kernel for
 * the task stacks, and so will legitimately appear in many positions within
 * the ISR stack. */
    #define portISR_STACK_FILL_BYTE    0xee
#else
    extern const uint32_t __freertos_irq_stack_top[];
    const StackType_t xISRStackTop = ( StackType_t ) __freertos_irq_stack_top;
#endif

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void ) __attribute__( ( weak ) );

/*-----------------------------------------------------------*/

/* Used to program the machine timer compare register. */
uint64_t ullNextTime = 0ULL;
const uint64_t * pullNextTime = &ullNextTime;
//const size_t uxTimerIncrementsForOneTick = ( size_t ) ( ( configCPU_CLOCK_HZ ) / ( configTICK_RATE_HZ ) ); /* Assumes increment won't go over 32-bits. */
const size_t uxTimerIncrementsForOneTick = ( size_t ) ( ( configCPU_CLOCK_HZ ) / ( configTICK_RATE_HZ ) ); /* Assumes increment won't go over 32-bits. */
////UBaseType_t const ullMachineTimerCompareRegisterBase = configMTIMECMP_BASE_ADDRESS;
uint32_t * pullMachineTimerCompareRegister = ( uint32_t * ) ( 0xc000f808 );
uint32_t * pullMachineTimerCompareRegister_lo = ( uint32_t * ) ( 0xc000f808 );
uint32_t * pullMachineTimerCompareRegister_hi = ( uint32_t * ) ( 0xc000f80c );
volatile uint32_t * ctrlReggister = (volatile  uint32_t *) ( 0xc000f810 );
unsigned int* led = (unsigned int*)0xc000fe00;
unsigned int led_val = 0;

/* Holds the critical nesting value - deliberately non-zero at start up to
 * ensure interrupts are not accidentally enabled before the scheduler starts. */
size_t xCriticalNesting = ( size_t ) 0xaaaaaaaa;
size_t * pxCriticalNesting = &xCriticalNesting;

/* Used to catch tasks that attempt to return from their implementing function. */
size_t xTaskReturnAddress = ( size_t ) portTASK_RETURN_ADDRESS;

/* Set configCHECK_FOR_STACK_OVERFLOW to 3 to add ISR stack checking to task
 * stack checking.  A problem in the ISR stack will trigger an assert, not call
 * the stack overflow hook function (because the stack overflow hook is specific
 * to a task stack, not the ISR stack). */
#if defined( configISR_STACK_SIZE_WORDS ) && ( configCHECK_FOR_STACK_OVERFLOW > 2 )
    #warning "This path not tested, or even compiled yet."

    static const uint8_t ucExpectedStackBytes[] =
    {
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, \
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, \
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, \
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, \
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE
    }; \

    #define portCHECK_ISR_STACK()    configASSERT( ( memcmp( ( void * ) xISRStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) == 0 ) )
#else /* if defined( configISR_STACK_SIZE_WORDS ) && ( configCHECK_FOR_STACK_OVERFLOW > 2 ) */
    /* Define the function away. */
    #define portCHECK_ISR_STACK()
#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

/*-----------------------------------------------------------*/

//#if ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 )

    void vPortSetupTimerInterrupt( void )
    {

		led_val++;
		//*led = led_val;
        uint32_t ulCurrentTimeHigh, ulCurrentTimeLow;
        //volatile uint32_t * const pulTimeHigh = ( volatile uint32_t * const ) ( ( configMTIME_BASE_ADDRESS ) + 4UL ); /* 8-byte type so high 32-bit word is 4 bytes up. */
        //volatile uint32_t * const pulTimeLow = ( volatile uint32_t * const ) ( configMTIME_BASE_ADDRESS );

        volatile uint32_t * pulTimeHigh = ( volatile uint32_t * )( 0xc000f804 ) ; /* 8-byte type so high 32-bit word is 4 bytes up. */
        volatile uint32_t * pulTimeLow = ( volatile uint32_t * ) ( 0xc000f800 );
        volatile uint32_t ulHartId;

        __asm volatile ( "csrr %0, mhartid" : "=r" ( ulHartId ) );

        //pullMachineTimerCompareRegister = ( volatile uint64_t * ) ( ullMachineTimerCompareRegisterBase + ( ulHartId * sizeof( uint64_t ) ) );
        //pullMachineTimerCompareRegister_hi = ( volatile uint32_t * ) ( 0xc000f80c );
        //pullMachineTimerCompareRegister_lo = ( volatile uint32_t * ) ( 0xc000f808 );
        //pullMachineTimerCompareRegister = ( volatile uint64_t * ) ( 0xc000f808 ); // for ASM file

		*ctrlReggister = 1; // clear interrupt
		//*ctrlReggister = 0; // debug : stop FRC

		vSendString( "change value\n" );

        do
        {
            ulCurrentTimeHigh = *pulTimeHigh;
            ulCurrentTimeLow = *pulTimeLow;
        } while( ulCurrentTimeHigh != *pulTimeHigh );

        ullNextTime = ( uint64_t ) ulCurrentTimeHigh;
        ullNextTime <<= 32ULL; /* High 4-byte word is 32-bits up. */
        ullNextTime |= ( uint64_t ) ulCurrentTimeLow;
        ullNextTime += ( uint64_t ) uxTimerIncrementsForOneTick;
        //*pullMachineTimerCompareRegister = ullNextTime;

        *pullMachineTimerCompareRegister_lo = (uint32_t)(0xffffffff);
        *pullMachineTimerCompareRegister_hi = (uint32_t)(ullNextTime >> 32ULL);
        *pullMachineTimerCompareRegister_lo = (uint32_t)(ullNextTime & 0xffffffff);

        /* Prepare the time to use after the next tick interrupt. */
	    char buf2[ 40 ];
    	sprintf( buf2, "uxTimerIncrementsForOneTick %d\n", uxTimerIncrementsForOneTick);
    	vSendString( buf2 );

        ullNextTime += ( uint64_t ) uxTimerIncrementsForOneTick;
		led_val++;
		//*led = led_val;
		//*led = 1;
		vSendString( "change value2\n" );

    }

//#endif /* ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIME_BASE_ADDRESS != 0 ) */
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
	led_val++;
	//*led = led_val;

	vSendString( "change value3\n" );
    extern void xPortStartFirstTask( void );

    #if ( configASSERT_DEFINED == 1 )
    {
        /* Check alignment of the interrupt stack - which is the same as the
         * stack that was being used by main() prior to the scheduler being
         * started. */
        configASSERT( ( xISRStackTop & portBYTE_ALIGNMENT_MASK ) == 0 );

        #ifdef configISR_STACK_SIZE_WORDS
        {
            memset( ( void * ) xISRStack, portISR_STACK_FILL_BYTE, sizeof( xISRStack ) );
        }
        #endif /* configISR_STACK_SIZE_WORDS */
    }
    #endif /* configASSERT_DEFINED */

    /* If there is a CLINT then it is ok to use the default implementation
     * in this file, otherwise vPortSetupTimerInterrupt() must be implemented to
     * configure whichever clock is to be used to generate the tick interrupt. */
    vPortSetupTimerInterrupt();

    #if ( ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 ) )
    {
        /* Enable mtime and external interrupts.  1<<7 for timer interrupt,
         * 1<<11 for external interrupt.  _RB_ What happens here when mtime is
         * not present as with pulpino? */
		vSendString( "change value6\n" );
        __asm volatile ( "csrw mie, %0" ::"r" ( 0x888 ) );
        //__asm volatile ( "csrw mstatus, %0" ::"r" ( 0x1888 ) );
    }
    #endif /* ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 ) */
	led_val++;
	//*led = led_val;
	*ctrlReggister = 3; // clear interrupt in FRC
	//*ctrlReggister = 0; // debug : stop FRC
	char buf[ 40 ];
	sprintf( buf, "%d : ", (int)*ctrlReggister);
    vSendString( buf );
    unsigned int mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
	sprintf( buf, "%x : ", mstatus);
    vSendString( buf );
    unsigned int mtvec;
    __asm__ volatile("csrr %0, mtvec" : "=r"(mtvec));
	sprintf( buf, "%x : ", mtvec);
    vSendString( buf );
 
 
	vSendString( "change value4\n" );
    xPortStartFirstTask();
	vSendString( "change value5\n" );

	*led = 1;

    /* Should not get here as after calling xPortStartFirstTask() only tasks
     * should be executing. */
    return pdFAIL;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented. */
    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/

void vPortDebufOutForException( void )
{

    vSendString( "exception " );
	char buf[ 40 ];
	sprintf( buf, "%d : ", (int)*ctrlReggister);
    vSendString( buf );

    unsigned int mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
	sprintf( buf, "mstatus %x : ", mstatus);
    vSendString( buf );

    unsigned int mtvec;
    __asm__ volatile("csrr %0, mtvec" : "=r"(mtvec));
	sprintf( buf, "mtvec %x : ", mtvec);
    vSendString( buf );

    unsigned int mcause;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
	sprintf( buf, "mcause %x : ", mcause);
    vSendString( buf );

    unsigned int mtval;
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
	sprintf( buf, "mtval %x : ", mtval);
    vSendString( buf );

    unsigned int mepc;
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
	sprintf( buf, "mepc %x : ", mepc);
    vSendString( buf );

    unsigned int mip;
    __asm__ volatile("csrr %0, mip" : "=r"(mip));
	sprintf( buf, "mip %x : \n", mip);
    vSendString( buf );

    for( ; ; )
    {
    }

}

void vPortDebufOutForInterrupt( void )
{

    vSendString( "interrupt " );
	char buf[ 40 ];
	sprintf( buf, "%d : ", (int)*ctrlReggister);
    vSendString( buf );

    unsigned int mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
	sprintf( buf, "mstatus %x : ", mstatus);
    vSendString( buf );

    unsigned int mtvec;
    __asm__ volatile("csrr %0, mtvec" : "=r"(mtvec));
	sprintf( buf, "mtvec %x : ", mtvec);
    vSendString( buf );

    unsigned int mcause;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
	sprintf( buf, "mcause %x : ", mcause);
    vSendString( buf );

    unsigned int mtval;
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
	sprintf( buf, "mtval %x : ", mtval);
    vSendString( buf );

    unsigned int mepc;
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
	sprintf( buf, "mepc %x : ", mepc);
    vSendString( buf );

    unsigned int mip;
    __asm__ volatile("csrr %0, mip" : "=r"(mip));
	sprintf( buf, "mip %x : \n", mip);
    vSendString( buf );

    for( ; ; )
    {
    }

}

