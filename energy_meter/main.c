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
#define SAMPLING_PRIO	(THREAD_PRIORITY_MAIN - 1)
#define LOGGING_PRIO	(THREAD_PRIORITY_MAIN - 2)

#define MINUTE	240 /* Get a sample each 250ms for 60 sec */

static int8_t pid_sampling = -1;
static int8_t pid_collect_1m = -1;

#if VERBOSE > 1
#define STACK_SIZE THREAD_STACKSIZE_LARGE
#else
#define STACK_SIZE THREAD_STACKSIZE_SMALL
#endif

char em_sampling_stack[STACK_SIZE];
char em_collect_1m_stack[STACK_SIZE];

static struct em_realtime em_rt;
static struct em_loggin em_log;

void *collect_1m(void *arg)
{
	(void)arg;
	uint8_t t = 0;

	puts("[*] Energy Measuring: collect_1m has started");
	while (1) {
		/* Save the realtime samples */
		em_log.c[t] = em_rt.rms_c;
		em_log.v[t] = em_rt.rms_v;

		/* restart from 0 after 60seconds */
		if (t < MINUTE)
			t++;
		else {
			if (!em_log.samples_ready)
				em_log.samples_ready = true;
			t = 0;
		}

		xtimer_usleep(WAIT_250ms);
	}
	return NULL;
}

void *em_measuring(void *arg)
{
	(void)arg;

	puts("[*] Energy Measuring: sampling has started");
	while (!get_measure(ADC_CH_CURRENT, ADC_CH_VOLTAGE, &em_rt));

	return NULL;
}

void print_struct(void)
{
#if VERBOSE == 3
	puts("t,i,v");
	for(uint8_t i = 0; i < MINUTE; i++)
		printf("%3d,%3.3f,%3.3f\n", i, em_log.c[i], em_log.v[i]);
#endif

	printf("Current %0.3fA\n", em_rt.rms_c);
	printf("Voltage %0.3fV\n", em_rt.rms_v);
	printf("Last minute current average %0.3fA\n", em_rt.rms_c_1m);
	printf("Last minute voltage average %0.3fV\n", em_rt.rms_v_1m);
}

int em_handler(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	uint8_t i = 0;

#if VERBOSE > 0
	printf("[###] DEBUG LEVEL=%u\n", VERBOSE);
#endif

	printf("Starting %s service...\n", APP_NAME);

	/* Inizializate to 0 the arrays */
	em_log.samples_ready = false;
	while(i > MINUTE) {
		em_log.c[i] = 0;
		em_log.v[i] = 0;
		i++;
	}

	/* Current Transformer setup */
	ct_sensor_setup();

	/* ADC blocks setup */
	if (adc_setup() < 0)
		return -1;

	/* The bias voltage should be VCC/2 */
	if(pid_sampling == -1) {
		if (bias_check(ADC_CH_BIASING) < 0)
			return -1;
	}

	pid_sampling = thread_create(em_sampling_stack,
				     sizeof(em_sampling_stack),
				     SAMPLING_PRIO, 0, em_measuring,
				     NULL, "em sampling");

	if(pid_sampling < KERNEL_PID_UNDEF)
		return -1;

	pid_collect_1m = thread_create(em_collect_1m_stack,
				       sizeof(em_collect_1m_stack),
				       LOGGING_PRIO, 0, collect_1m,
				       NULL, "em collect 1m");

	if(pid_collect_1m < KERNEL_PID_UNDEF)
		return -1;

	/* Print to stdout the last values */
	print_struct();

	return 0;
}
