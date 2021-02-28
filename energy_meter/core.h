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

#define SAMPLE_FREQUENCY	(AC_F * SAMPLE_UNIT)
#define ADC_US_SLEEP		(K * K * 1/(SAMPLE_FREQUENCY))
#define BIAS_OFFSET		((1 << ADC_BIT) >> 1)

/* Bias boundary: ANALOG_VCC ± 2,5% */
#define V_MIN	((ANALOG_VCC * 0.5) - ((ANALOG_VCC / 200) * 2.5))
#define V_MAX	((ANALOG_VCC * 0.5) + ((ANALOG_VCC / 200) * 2.5))

/* Bias measure count */
#define BIAS_AVARAGE 10U

#define MINUTE	240 /* Get a sample each 250ms */

#define UNSUED __attribute__((__unused__))

#ifndef M_SQRT2
#define M_SQRT2		1.41421356237309504880
#endif

/* @em_realtime contains all notable datas:
 *  rms_c: current real time values, update each sec
 *  rms_v: voltage real time values, update each sec
 *  rms_c_1m: avg of last minute average, update each 60 sec
 *  rms_v_1m: avg of last minute average, update each 60 sec
 *  samples_ready: true when  is ready.
 */
struct em_realtime {
	float rms_c, rms_v;
	float rms_c_1m, rms_v_1m;
	bool samples_ready;
};

/* @struct em_loggin contain collected datas
 *  c[MINUTE]: arrary where to store the current values
 *  v[MINUTE]: arrary where to store the voltage values
 *  samples_ready: true when array is fully initialized.
 */
struct em_loggin {
	float c[MINUTE], v[MINUTE];
	bool samples_ready;
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
 * ch_I: ADC channel where to read the analog input of current
 * ch_V: ADC channel where to read the analog input of voltage
 * em: struct em_realtime pointer
 *
 * Return 0 on a success
 */
int get_measure(const uint8_t ch_I, const uint8_t ch_V, struct em_realtime *em);

/* @bias_check: Check if the biasing voltage is ready and good. The bias check
 *              gets a mesure of the DC voltage by input channel ch_B.
 *
 * Returns 0 on a success
 */
int bias_check(const uint8_t ch_B);

#endif
