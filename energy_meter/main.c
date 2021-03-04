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

static int adc_offset;
static int8_t pid_sampling = -1;
static int8_t pid_collect_1m = -1;
static int8_t pid_collect_10m = -1;

#if VERBOSE > 1
#define STACK_SIZE THREAD_STACKSIZE_LARGE
#else
#define STACK_SIZE THREAD_STACKSIZE_SMALL
#endif

char em_sampling_stack[STACK_SIZE];
char em_collect_1m_stack[STACK_SIZE];
char em_collect_10m_stack[STACK_SIZE];

static struct em_realtime em_rt;
static struct em_loggin em_log;

static void *collect_10m(UNSUED void *arg)
{
	uint8_t t = 0; /* it counts 10 minute */

	puts("[*] Energy Measuring: collect_10m has started");
	while (1) {
		xtimer_usleep(WAIT_2500ms);

		/* Save the realtime samples */
		em_log.c10m[t] = em_rt.rms_c;
		em_log.v10m[t] = em_rt.rms_v;
		t++;

		/* Restart from 0 after 10 minutes so overwrites the old data
		 * samples
		 */
		if (t == MINUTE10) {
			/* Each 10s set 10m stats as ready. */
			if (!em_log.samples_10m_ready)
				em_log.samples_10m_ready = true;
			t = 0;
		}
	}

	return NULL;
}

static void *collect_1m(UNSUED void *arg)
{
	uint8_t t = 0; /* it counts 60 seconds */

	puts("[*] Energy Measuring: collect_1m has started");
	while (1) {
		xtimer_usleep(WAIT_250ms);

		/* Save the realtime samples */
		em_log.c[t] = em_rt.rms_c;
		em_log.v[t] = em_rt.rms_v;
		t++;

		/* Restart from 0 after 60 seconds so overwrites the old data
		 * samples
		 */
		if (t == MINUTE) {
			/* Each 60s  set 60s stats as ready. */
			if (!em_log.samples_1m_ready)
				em_log.samples_1m_ready = true;
			t = 0;
		}
	}

	return NULL;
}

static void *em_measuring(UNSUED void *arg)
{
	puts("[*] Energy Measuring: sampling has started");
	while (!get_measure(ADC_CH_CURRENT, ADC_CH_VOLTAGE, &em_rt, adc_offset));

	return NULL;
}

static  void print_data(void)
{
	uint8_t i;

#if VERBOSE == 3
	/* Print into csv format */
	puts("Last 60s samples:\n id;Current;Voltage");
	for(i = 0; i < MINUTE; i++)
		printf("%3d; %3.3f; %3.3f\n", i, em_log.c[i], em_log.v[i]);
	puts("Last 10m samples:\n id;Current;Voltage");
	for(i = 0; i < MINUTE10; i++)
		printf("%3d; %3.3f; %3.3f\n", i, em_log.c10m[i], em_log.v10m[i]);
#endif

	printf("Current %0.3fA\n", em_rt.rms_c);
	printf("Voltage %0.3fV\n", em_rt.rms_v);

	for(i = 0; i < MINUTE && em_log.samples_1m_ready; i++) {
		/* Note: MINUTE == MINUTE10 */
		if(em_log.samples_10m_ready) {
			em_rt.rms_c_10m += em_log.c10m[i];
			em_rt.rms_v_10m += em_log.v10m[i];
		}
		em_rt.rms_c_1m += em_log.c[i];
		em_rt.rms_v_1m += em_log.v[i];
	}

	if (em_log.samples_1m_ready) {
		em_rt.rms_c_1m /= MINUTE;
		em_rt.rms_v_1m /= MINUTE;
		printf("last minute current average %0.3fa\n", em_rt.rms_c_1m);
		printf("last minute voltage average %0.3fv\n", em_rt.rms_v_1m);
	}

	if (em_log.samples_10m_ready) {
		em_rt.rms_c_10m /= MINUTE10;
		em_rt.rms_v_10m /= MINUTE10;
		printf("last 10 minute current average %0.3fa\n", em_rt.rms_c_10m);
		printf("last 10 minute voltage average %0.3fv\n", em_rt.rms_v_10m);
	}
}

int em_init(void)
{
	int adc_offset;
#if VERBOSE > 0
	printf("[###] DEBUG LEVEL=%u\n", VERBOSE);
#endif

	printf("Starting %s service...\n", APP_NAME);

	/* Inizializate to 0 the arrays */
	em_log.samples_1m_ready = false;
	em_log.samples_10m_ready = false;

	/* Current Transformer setup */
	ct_sensor_setup();

	/* ADC blocks setup */
	if (adc_setup() < 0)
		return -1;

	/* The bias voltage should be VCC/2 */
	if(pid_sampling == -1) {
		if (bias_check(ADC_CH_BIASING, &adc_offset) < 0)
			return -1;
	}

	pid_sampling = thread_create(em_sampling_stack,
				     sizeof(em_sampling_stack),
				     SAMPLING_PRIO, 0, em_measuring,
				     NULL, "em sampling");

	if(pid_sampling < KERNEL_PID_UNDEF)
		return -1;

	/* XXX: The logs stats are using a  poor design of a circular buffer */
	pid_collect_1m = thread_create(em_collect_1m_stack,
				       sizeof(em_collect_1m_stack),
				       LOGGING_PRIO, 0, collect_1m,
				       NULL, "em collect 1m");

	if(pid_collect_1m < KERNEL_PID_UNDEF)
		return -1;

	pid_collect_10m = thread_create(em_collect_10m_stack,
				       sizeof(em_collect_10m_stack),
				       LOGGING_PRIO, 0, collect_10m,
				       NULL, "em collect 10m");

	if(pid_collect_10m < KERNEL_PID_UNDEF)
		return -1;

	return 0;
}

int em_handler(UNSUED int argc, UNSUED char *argv[])
{
	/* Print to stdout the last values */
	print_data();

	return 0;
}
