/**
 * Author:	Giuseppe Tipaldi
 * Created:	2019
 **/

#ifndef CORE_H
#define CORE_H

#include <stdio.h>
#include <stdbool.h>

#define K		1000

#define AC_F		50U	/* AC Frequency */

/*
 * Avoid rail-to-rail waveform, such expedient should reduce the no-linearity
 * contribs. I instead to use full range 3.3V, it uses 3V
 */
#define ANALOG_IN_VPP	3U

/* Voltage REF */
#define ANALOG_VCC	3.3

/* ADC CHANNELS */
#define ADC_CH_CURRENT	0U	/* CN8: A0 (PA0) Analog Input */
#define ADC_CH_VOLTAGE	5U	/* CN8: A5 (PA5) Analog Input */
#define ADC_CH_BIASING	3U	/* CN8: A3 (PA3) Analog Input */


#if BIT == 0
#define ADC_RES ADC_RES_12BIT
#define ADC_BIT 12
#elif BIT == 1
#define ADC_RES ADC_RES_10BIT
#define ADC_BIT 10
#elif BIT == 2
#define ADC_RES ADC_RES_8BIT
#define ADC_BIT 8
#else
#error "WRONG ADC RESOLUTION"
#endif

/* Delays */
#define WAIT_100ms	(100LU * US_PER_MS)	/* delay of 100 ms */
#define WAIT_250ms	(250LU * US_PER_MS)	/* delay of 250 ms */
#define WAIT_500ms	(500LU * US_PER_MS)	/* delay of 500 ms */
#define WAIT_1000ms	(1000LU * US_PER_MS)	/* delay of 1 s */
#define WAIT_2500ms	(2500LU * US_PER_MS)	/* delay of 2.5 s */
#define WAIT_15000ms	(15000LU * US_PER_MS)	/* delay of 15 s */

#define SAMPLE_FREQUENCY	(AC_F * SAMPLE_UNIT)
#define ADC_US_SLEEP		(K * K * 1/(SAMPLE_FREQUENCY))
#define BIAS_OFFSET		((1 << ADC_BIT) >> 1)

/* Bias boundary: ANALOG_VCC ± 2,5% */
#define V_MIN	((ANALOG_VCC * 0.5) - ((ANALOG_VCC / 200) * 2.5))
#define V_MAX	((ANALOG_VCC * 0.5) + ((ANALOG_VCC / 200) * 2.5))

/* Bias measure count */
#define BIAS_AVARAGE 10U

#define MINUTE 240 /* Get a sample each 250ms */
#define MINUTE10 240 /* Get a sample each 2.5s  */
#define MINUTE60 240 /* Get a sample each 15s */

#define UNSUED __attribute__((__unused__))

#ifndef M_SQRT2
#define M_SQRT2		1.41421356237309504880
#endif

/* @em_realtime contains all notable datas:
 *  rms_c: current real time values, update each sec
 *  rms_v: voltage real time values, update each sec
 *  rms_c_1m: avg of last minute, updated each 250ms sec
 *  rms_v_1m: avg of last minute, updated each 250ms sec
 *  rms_c_10m: avg of last minute, updated each 60 sec
 *  rms_v_10m: avg of last minute, updated each 60 sec
 *  rms_c_60m: avg of last minute, updated each 60 sec
 *  rms_v_60m: avg of last minute, updated each 60 sec
 */
struct em_realtime {
	float rms_c, rms_v;
	float rms_c_1m, rms_v_1m;
	float rms_c_10m, rms_v_10m;
	float rms_c_60m, rms_v_60m;  
};

/* @struct em_loggin contain collected datas
 *  c[MINUTE]: arrary where to store the current values
 *  v[MINUTE]: arrary where to store the voltage values
 *  c10m[MINUTE10]: arrary where to store the current values
 *  v10m[MINUTE10]: arrary where to store the voltage values
 *  c60m[MINUTE60]: arrary where to store the current values
 *  v60m[MINUTE60]: arrary where to store the voltage values
 *  samples_1m_ready: true if last 60 seconds values are fully initialized.
 *  samples_10m_ready: true if last 10 minute values are fully initialized.
 *  samples_60m_ready: true if last 10 minute values are fully initialized.
 */
struct em_loggin {
	float c[MINUTE], v[MINUTE];
	float c10m[MINUTE10], v10m[MINUTE10];
	float c60m[MINUTE60], v60m[MINUTE60];
	bool samples_1m_ready, samples_10m_ready, samples_60m_ready;
};

/*
 * -------API-------
 */

/* @ct_sensor_setup: setup and initialize the current transformer (CT) block.
 *
 * Returns 0 on a success
 */
int ct_sensor_setup(void);

/* @adc_setup: setup and initialize the adc block.
 *
 * Returns 0 on a success
 */
int adc_setup(void);

/* @get_measure: Main acquisition loop, it collects a number of current and
 *               voltage samples, compute rms average value and store it.
 *               It needs of adc's channel of current, voltage, and pointer
 *               of struct em_realtime.
 *
 *               WARNING: Only the current reading has been tested.
 * @param[in]: ch_I: ADC channel where to read the analog input of current
 * @param[in]: ch_V: ADC channel where to read the analog input of voltage
 * @param[in]: em: struct where to store value
 * @param[in]: adc_offset: adc offset value
 *
 * Return 0 on a success
 */
int get_measure(const uint8_t ch_I, const uint8_t ch_V,
		struct em_realtime *em, const int adc_offset);

/* @bias_check: Check if the biasing voltage is ready and good. The bias check
 *              gets a mesure of the DC voltage by input channel ch_B.
 * @param[in]: ch_B: ADC channel
 * @param[out]: adc_offset: where to store the adc offset
 * Returns 0 on a success
 */
int bias_check(const uint8_t ch_B, int *adc_offset);

#endif
