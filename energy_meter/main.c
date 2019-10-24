#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define APP_NAME "EnergMeter"

int em_handler(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	printf("Welcome to %s\n", APP_NAME);

	return 0;
}
