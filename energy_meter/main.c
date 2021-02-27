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

static int8_t pid_sampling = -1;
static int8_t pid_logging = -1;

#if VERBOSE > 1
#define STACK_SIZE THREAD_STACKSIZE_LARGE
#else
#define STACK_SIZE THREAD_STACKSIZE_SMALL
#endif

char em_sampling_stack[STACK_SIZE];
char em_logging_stack[STACK_SIZE];

static struct em_realtime em_rt;
static struct em_loggin em_log;

void *em_log_60s(void *arg)
{
	(void)arg;
	uint8_t t = 0;

	puts("[*] Energy Measuring: log minute has started");
	while (1) {
		em_log.c[t] = em_rt.rms_c;
		em_log.v[t] = em_rt.rms_v;

		if(t == 59) {

			while(t > 0) {
				em_rt.rms_c_1m += em_log.c[t];
				em_rt.rms_v_1m += em_log.v[t];
				t--;
			}

			em_rt.rms_c_1m /= 60;
			em_rt.rms_v_1m /= 60;
			em_rt.log_1m_ready = true;
#if VERBOSE == 3
			/* Precision: 2 digit after the comma */
			for(uint8_t i = 0; i < 60; i++)
				printf("\t id=%d I=%.2f V=%.2f\n",
				       i, em_log.c[i], em_log.v[i]);
#endif
		}

		t++;
		xtimer_usleep(WAIT_1000ms);
	}

	puts("[*] Energy Measuring: log minute has started");
	return NULL;
}

void *em_measuring(void *arg)
{
	(void)arg;

	puts("[*] Energy Measuring: sampling has started");
	xtimer_ticks32_t last = xtimer_now();
	while (1) {

		if (get_measure(ADC_CH_CURRENT, ADC_CH_VOLTAGE, &em_rt) < 0)
			return NULL;

		xtimer_periodic_wakeup(&last, WAIT_1000ms);
	};

	return NULL;
}

void print_struct(void)
{
	printf("Current %0.3fA\n", em_rt.rms_c);
	printf("Voltage %0.3fV\n", em_rt.rms_v);
	if(em_rt.log_1m_ready) {
		printf("Last minute current average %0.3fA\n", em_rt.rms_c_1m);
		printf("Last minute voltage average %0.3fV\n", em_rt.rms_v_1m);
	}

}

int em_handler(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	int8_t ret;

#if VERBOSE > 0
	printf("[###] DEBUG LEVEL=%u\n", VERBOSE);
#endif

	printf("Starting %s service...\n", APP_NAME);

	/* Current Transformer setup */
	ct_sensor_setup();

	/* ADC blocks setup */
	ret = adc_setup();
	if (ret < 0)
		return -1;

	/* The bias voltage should be VCC/2 */
	if(pid_sampling == -1) {
		ret = bias_check(ADC_CH_BIASING);
		if (ret < 0)
			return -1;
	}

	if(pid_sampling == -1) {

		pid_sampling = thread_create(em_sampling_stack,
					     sizeof(em_sampling_stack),
					     SAMPLING_PRIO, 0, em_measuring,
					     NULL, "em sampling");

		if(pid_sampling < KERNEL_PID_UNDEF)
			return -1;
		else
			em_rt.log_1m_ready = false;

	} else {
		printf("[!] Energy Measuring: sampling already started, pid=%d\n",
			pid_sampling);
	}

	if (pid_sampling > 0 && pid_logging == -1) {

		pid_logging = thread_create(em_logging_stack,
				   sizeof(em_logging_stack),
				   LOGGING_PRIO, 0, em_log_60s,
				   NULL, "em log minute");

		if(pid_logging < KERNEL_PID_UNDEF)
			return -1;
	} else {
		printf("[!] Energy Measuring: log minute already started, pid=%d\n",
		       pid_logging);
	}

	/* Print to stdout the last values */
	print_struct();

	return 0;
}
