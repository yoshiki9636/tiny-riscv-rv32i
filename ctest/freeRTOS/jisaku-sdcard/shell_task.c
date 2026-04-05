#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS_CLI.h"

#define INPUT_BUFFER_SIZE 256
static char input[INPUT_BUFFER_SIZE];
static char output[256];

void uprint( const char* buf, int length, int ret );

char uart_getchar() {
    volatile unsigned int* rx_char = (unsigned int*)0xc000fc0c;

	unsigned int char_reg;
    while (1) {
        char_reg = *rx_char;
        while ((char_reg & 0xf00) == 0) {
            char_reg = *rx_char;
        }
        if ((char_reg & 0x100) != 0) {
            break;
        }
    }
    char c = (char)(char_reg & 0xff);
    char_reg = *rx_char;

    return c;

}

void vShellTask(void *pvParameters)
//void vShellTask()
{
    int idx = 0;
    char c;

    while (1)
    {
        c = uart_getchar();

        if (c == '\r' || c == '\n')
        {
            input[idx] = '\0';
            BaseType_t more;

            do {
                more = FreeRTOS_CLIProcessCommand(
                    input,
                    output,
                    sizeof(output)
                );

                //printf("%s", output);
				int len = strlen(output);
				uprint( output, len, 0 );

            //} while (strlen(output) > 0);
            } while (more == pdTRUE);

            idx = 0;
            //printf("\n> ");
			uprint( "", 0, 2 );
        }
        else if (idx < INPUT_BUFFER_SIZE - 1)
        {
            input[idx++] = c;
			//uprint( &c, 1, 0 );
            //uart_putchar(c); // エコーバック
        }
    }
}





