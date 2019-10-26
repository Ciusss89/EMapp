#include <unistd.h>
#include <string.h>
#include <stdio.h>

/* RIOT APIs */
#include "periph/adc.h"
#include "xtimer.h"
#include "timex.h"

#include "em.h"

#define APP_NAME	"EnergMeter"
#define VERBOSE		1

#define ADC_CH_CURRENT	1U			/* CN8: A1 (PA1) Analog Input */
#define ADC_CH_VOLTAGE	2U			/* CN8: A2 (PA2) Analog Input */
#define ADC_RES		ADC_RES_12BIT		/* ADC resolution */
#define WAIT_1msec	(1LU * US_PER_MS)	/* 1 ms */

#define BIAS_OFFSET	2048
#define SAMPLE		20U

/*
 * The The GPIOs can sink or source up to ±8 mA  and sink or
 * source up to ±20 mA except PC13, PC14 and PC15 which can
 * sink or source up to ±3mA.
 */
#define MCU_MAX_CURRENT_SINK	20U /* 20mA */

/* Primary max current [A] */
const float p_max_cur = (MAX_CURRENT * M_SQRT2);

/* Secondary max current [A] */
const float s_max_cur = (p_max_cur / CT_RATIO);

/* Burden resitor [Ω] */
const float bur_resistor = ((ANALOG_IN_VPP * 0.5) / s_max_cur);

static int ct_sensor_setup(void)
{
#ifdef VERBOSE
	printf("[*] CT sensor setup:\n");
	printf("\t Max measurable current: %uA\n", MAX_CURRENT);
	printf("\t Max primary peak current: %fA\n", p_max_cur);
	printf("\t Max secondary peak current: %fA\n", s_max_cur);
	printf("\t Burden resistor: %fΩ\n", bur_resistor);
#endif

	if((s_max_cur *K) > MCU_MAX_CURRENT_SINK) {
		printf("[!] Max secondary peak current exceeds the gpio sink limit.\n");
		return -1;
	}

	return 0;
}

static int adc_setup(void)
{
	int ret;

	if(ADC_CH_CURRENT > ADC_NUMOF) {
		printf("[!] ADC channel (%u) out of range\n", ADC_CH_CURRENT);
		goto err;
	} else if (ADC_CH_VOLTAGE > ADC_NUMOF){
		printf("[!] ADC channel (%u) out of range\n", ADC_CH_VOLTAGE);
		goto err;
	}

	ret = adc_init(ADC_LINE(ADC_CH_CURRENT));
	if(ret < 0) {
		printf("[!] Initialization of ADC_LINE(%u) failed\n",
		       ADC_CH_CURRENT);
		goto err;
	}

	ret = adc_init(ADC_LINE(ADC_CH_VOLTAGE));
	if(ret < 0) {
		printf("[!] Initialization of ADC_LINE(%u) failed\n",
		       ADC_CH_VOLTAGE);
		goto err;
	}

	return 0;
err:
	return -1;
}

static int get_measure(int ch_I, int ch_V)
{
	xtimer_ticks32_t last = xtimer_now();
	int16_t buff_I[SAMPLE], buff_V[SAMPLE], v=0, c=0;
	uint8_t i = 0;

	/*
	 * SAMPLING :
	 *
	 * - To seample an AC waveform, the inpunt needs to be biased at Vcc/2.
	 *   This translates to a digital offset value of approximately half of
	 *   the ADC resolution
	 *
	 * - I don't want use a ratil-to-rail a conversation, the burden
	 *   resistor is computed to have a 3 V peak to peak waveform. it's
	 *   centered around 1.65V
	 *
	 * - AC has a frequency of 50Hz, we capture 20 samples each 1 m
	 *
	 * - I have to remove the bias offset
	 *
	 * My case:
	 *  [1] VCC = 3.3 -> bias value is 1.65 V
	 *  [2] ADC_RES = 12 bit  (4096)
	 *  [3] Bias Offset = 2048
	 */

	do {
		buff_I[i] = adc_sample(ADC_LINE(ch_I), ADC_RES);
		buff_V[i] = adc_sample(ADC_LINE(ch_V), ADC_RES);

		if((buff_I[i] < 0) || (buff_V[i] < 0)) {
			printf("[!] Resolution %u not supported\n", ADC_RES);
			return -1;
		}

		buff_I[i] -= BIAS_OFFSET;
		buff_V[i] -= BIAS_OFFSET;

		v += buff_V[i];
		c += buff_I[i];

		i++;
		xtimer_periodic_wakeup(&last, WAIT_1msec);
	} while (i < SAMPLE);

	//printf("media-> V=%d I=%d\n",v/20, c/20);

	return 0;
}

int em_handler(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	int ret;

	ct_sensor_setup();

	ret = adc_setup();
	if (ret < 0)
		return -1;

	ret = get_measure(ADC_CH_CURRENT, ADC_CH_VOLTAGE);
	if (ret < 0)
		 return -1;

	return 0;
}
