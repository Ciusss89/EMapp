#include <stdio.h>
#include <string.h>

#include "shell.h"

int main(void)
{
    printf("RIOT on a %s board, MCU %s\n\r", RIOT_BOARD, RIOT_MCU);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
