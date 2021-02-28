/**
 * Author:	Giuseppe Tipaldi
 * Created:	2019
 **/

#include <unistd.h>
#include <stdio.h>

#include "shell.h"
#include "energy_meter/em.h"

static const shell_command_t cmds[] = {
	{ "em", "em - energy meter application", em_handler },
	{ NULL, NULL, NULL }
};

int main(void)
{
	printf("RIOT on a %s board, MCU %s\n\r", RIOT_BOARD, RIOT_MCU);

	/* Start energy meter core */
	if (em_init() < 0) {
		puts("[!] Fatal ERROR");
		return -1;
	}

	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(cmds, line_buf, SHELL_DEFAULT_BUFSIZE);

	return 0;
}
