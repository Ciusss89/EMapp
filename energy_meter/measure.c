/**
 * Author:	Giuseppe Tipaldi
 * Created:	2019
 **/

#include "core.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* RIOT APIs */
#include "periph/adc.h"
#include "xtimer.h"
#include "timex.h"

/* current transformer specs */
#include "ct.h"

/* Primary peak current [A] */
const float p_max_cur = (RMS_MAX_CURRENT *  M_SQRT2);

/* Secondary peak current [A] */
const float s_max_cur = (p_max_cur / CT_RATIO);

/* Burden resitor [Ω] */
const float bur_resistor = ((ANALOG_IN_VPP * 0.5) / s_max_cur);

/* The RMS values computed by get_measure loop are related to
 * ADC resolution scale. (The value is included in between range: 0-2^ADC_BIT)
 */
const float tmp_adc_scale_factor = ((1 << ADC_BIT) / ANALOG_IN_VPP);
const float adc_scale_factor = (1 / tmp_adc_scale_factor);

const float tmp_adc_scale_bias = ((1 << ADC_BIT) / ANALOG_VCC);
const float adc_scale_bias = (1 / tmp_adc_scale_bias);

int ct_sensor_setup(void)
{
#if VERBOSE >= 1
	printf("[*] CT sensor setup:\n");
	printf("\t RMS MAX current: %uA\n", RMS_MAX_CURRENT);
	printf("\t Max primary peak current: %fA\n", p_max_cur);
	printf("\t Max secondary peak current: %fA\n", s_max_cur);
	printf("\t Burden resistor: %fΩ\n", bur_resistor);
#endif

	if(RMS_MAX_CURRENT > CT_MAX_INPUT) {
		printf("[!] RMS primary current exceeds the CT limit.\n");
		return -1;
	}

	if((s_max_cur * K) > MCU_MAX_CURRENT_SINK) {
		printf("[!] Max secondary peak current exceeds the gpio sink limit.\n");
		return -1;
	}

	return 0;
}

int adc_setup(void)
{
	int ret;

	if(ADC_CH_CURRENT > ADC_NUMOF) {
		printf("[!] ADC channel (%u) out of range\n", ADC_CH_CURRENT);
		goto err;
	} else if (ADC_CH_VOLTAGE > ADC_NUMOF) {
		printf("[!] ADC channel (%u) out of range\n", ADC_CH_VOLTAGE);
		goto err;
	} else if (ADC_CH_BIASING > ADC_NUMOF) {
		printf("[!] ADC channel (%u) out of range\n", ADC_CH_BIASING);
		goto err;
	}

	ret = adc_init(ADC_LINE(ADC_CH_BIASING));
	if(ret < 0) {
		printf("[!] Initialization of ADC_LINE(%u) failed\n",
		       ADC_CH_CURRENT);
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

#if VERBOSE >= 1
	printf("[*] ADC setup:\n");
	printf("\t ADC bits: %u\n", ADC_BIT);
	printf("\t ADC bias offset: %u\n", BIAS_OFFSET);
	printf("\t ADC scale factor: %f\n", adc_scale_factor);
	printf("\t ADC sampling frequency: %uHZ\n", SAMPLE_FREQUENCY);
	printf("\t ADC gets [%u] sample each %u usec\n", SAMPLE_UNIT, ADC_US_SLEEP);
#endif

	return 0;
err:
	return -1;
}

int get_measure(const uint8_t ch_I, const uint8_t ch_V, struct em_realtime *em)
{
	uint32_t sum_squared_c = 0, sum_squared_v = 0;
	int16_t I[SAMPLE_UNIT], V[SAMPLE_UNIT];
	float rms_in_c = 0, rms_in_v = 0;
	uint8_t i = 0;

#if VERBOSE == 2
	const uint32_t start = xtimer_now_usec();;
#endif
	do {
		/* SAMPLING: Remove dc offset */
		I[i] = (adc_sample(ADC_LINE(ch_I), ADC_RES) - (BIAS_OFFSET));
		V[i] = (adc_sample(ADC_LINE(ch_V), ADC_RES) - (BIAS_OFFSET));

		sum_squared_c += (I[i] * I[i]);
		sum_squared_v += (V[i] * V[i]);

		i++;
		xtimer_usleep(ADC_US_SLEEP);
	} while (i < SAMPLE_UNIT);
#if VERBOSE == 2
	const uint32_t stop = xtimer_now_usec();
	for(i = 0; i < SAMPLE_UNIT; i++)
		 printf("[*] adc_samples_raw[%u] I=%d V=%d\n", i, I[i], V[i]);
#endif

	/* They should be equal to the RMS voltage reads by ADC */
	rms_in_c = sqrt(sum_squared_c / (SAMPLE_UNIT)) * adc_scale_factor;
	rms_in_v = sqrt(sum_squared_v / (SAMPLE_UNIT)) * adc_scale_factor;

#if VERBOSE == 2
	printf("[*] Acquisition time %lu usec, RMS voltage for AC current(%fV), for AC voltage (%fV)\n",
			(stop - start), rms_in_c, rms_in_v);
#endif

	/* save the rms measure of voltage and current */
	em->rms_c = ((rms_in_c * CT_RATIO) / bur_resistor);
	em->rms_v = 230; /* I don't have the voltage probe yet */
	(void)rms_in_v;  /* workaround to silent the -Werror=unused-but-set-variable*/

	return 0;
}

int bias_check(const uint8_t ch)
{
	float ret;
	float val = 0;
	xtimer_ticks32_t last = xtimer_now();

	/* Bias check */
	for(uint8_t i = 0; i < BIAS_AVARAGE; i++) {
		ret = adc_sample(ADC_LINE(ch), ADC_RES);
		ret *= adc_scale_bias;
		val += ret;
		xtimer_periodic_wakeup(&last, WAIT_100ms);
	}

	val /= BIAS_AVARAGE;

#if VERBOSE >= 1
	printf("[*] Calibration loop:\n");
	printf("\t ADC(%u) bias boundary: [%0.2f-%0.2f]V\n", ch, V_MIN, V_MAX);
	printf("\t Bias readed voltage: %.4fV\n", val);
#endif

	if (ret < V_MIN || ret > V_MAX) {
		printf("[!] Bias check fails for ADC ch(%u), val=%.4fV\n", ch, ret);
		goto err;
	}

	return 0;
err:
	return -1;
}
