/**
 * Author:	Giuseppe Tipaldi
 * Created:	2019
 **/

#include <stdbool.h>

#include "em.h"
#include "core.h"

/* RIOT APIs */
#include "thread.h"
#include "xtimer.h"
#include "timex.h"

#define APP_NAME	"EnergMeter"

char em_thread_stack[THREAD_STACKSIZE_DEFAULT];

static bool em_measuring_running = false;

static struct em_data p;

void *em_measuring(void *arg)
{
	int8_t pid;
	(void)arg;

	em_measuring_running = true;
	printf("[*] Energy Measuring has started\n");

	xtimer_ticks32_t last = xtimer_now();
	while(1){
		pid = get_measure(ADC_CH_CURRENT, ADC_CH_VOLTAGE, &p);
		if (pid < 0)
			return NULL;
		xtimer_periodic_wakeup(&last, WAIT_1000ms);
	};

	return NULL;
}

void print_struct(void)
{
	printf("Current %0.3fA\n", p.rms_c);
	printf("Voltage %0.3fV\n", p.rms_v);
}

int em_handler(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	int8_t ret, p1;

#if VERBOSE > 0
	printf("[###] DEBUG LEVEL=%u\n", APP_NAME, VERBOSE);
#endif

	printf("Starting %s service...\n", APP_NAME);

	/* Current Transformer setup */
	ct_sensor_setup();

	/* ADC blocks setup */
	ret = adc_setup();
	if (ret < 0)
		return -1;

	/* The bias voltage should be VCC/2 */
	if(!em_measuring_running) {
		ret = bias_check(ADC_CH_BIASING);
		if (ret < 0)
			return -1;
	}

	if(!em_measuring_running) {
	 	p1= thread_create(em_thread_stack,
				  sizeof(em_thread_stack),
				  THREAD_PRIORITY_MAIN - 1,
				  THREAD_CREATE_STACKTEST,
				  em_measuring, NULL, "em get");
		if(p1 < KERNEL_PID_UNDEF)
			return -1;
	} else {
		printf("[!] Energy Measuring already started\n");
	}

	/* Print to stdout the last values */
	print_struct();

	return 0;
}
