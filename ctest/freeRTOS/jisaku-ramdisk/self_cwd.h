#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "ff_stdio.h"
#include <string.h>
#include <stdio.h>

#define MAX_PATH 256
extern void vNormalizePath(char *out, const char *in);

