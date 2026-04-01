#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "ff_stdio.h"
#include <string.h>
#include <stdio.h>

#include "self_cwd.h"

extern char pcCurrentDir[];

void vNormalizePath(char *out, const char *in)
{
    char temp[MAX_PATH];
    if(in[0] == '/')
    {
        strncpy(temp, in, MAX_PATH);
    }
    else
    {
        snprintf(temp, MAX_PATH, "%s/%s", pcCurrentDir, in);
    }

    char *tokens[32];
    int count = 0;

    char *p = strtok(temp, "/");
    while(p != NULL && count < 32)
    {
        if(strcmp(p, "..") == 0)
        {
            if(count > 0) count--;
        }
        else if(strcmp(p, ".") != 0)
        {
            tokens[count++] = p;
        }
        p = strtok(NULL, "/");
    }

    strcpy(out, "/");
    for(int i = 0; i < count; i++)
    {
        strcat(out, tokens[i]);
        if(i != count - 1) strcat(out, "/");
    }
}
