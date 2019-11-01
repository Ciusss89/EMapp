
#ifndef CORE_H
#define CORE_H

#include <stdio.h>

#define VERBOSE		1U	/* VALUES: [0,1,2,3,4] */

#define RMS_MAX_CURRENT	7U	/* Maxinum Measurable Current */
#define K		1000

#define AC_F		50U	/* AC Frequency */

/*
 * I prefer don't have a rail-to-rail waveform, such expedient should
 * reduce the no-linearity contribs. I instead to set 3.3V I'll use 3V
 */
#define ANALOG_IN_VPP	3U

/* Voltage REF*/
#define ANALOG_VCC	3.3

/* ADC CHANNELS */
#define ADC_CH_CURRENT	0U	/* CN8: A0 (PA0) Analog Input */
#define ADC_CH_VOLTAGE	5U	/* CN8: A5 (PA5) Analog Input */
#define ADC_CH_BIASING	3U	/* CN8: A3 (PA3) Analog Input */

#define ADC_RES		ADC_RES_12BIT	/* ADC resolution */
#if ADC_RES == ADC_RES_12BIT
#define ADC_BIT 	12U
#elif ADC_RES == ADC_RES_10BIT
#define ADC_BIT		10U
#endif

/* Delays */
#define WAIT_100ms	(100LU * US_PER_MS)	/* delay of 100 ms */
#define WAIT_500ms	(500LU * US_PER_MS)	/* delay of 500 ms */
#define WAIT_1000ms	(1000LU * US_PER_MS)	/* delay of 1 s */

/*
 * Sempling:
 * - According to the NYQUEST roule 100Hz should be enaght, but I want at
 *   least 10 semples to compute a RMS value.
 * - Auto compute the adc sampling time in relation to SAMPLE_UNIT
 * - Auto compute the bias in relation to ADC_RES
 */
#define SAMPLE_UNIT		12U
#define SAMPLE_FREQUENCY	(AC_F * SAMPLE_UNIT)
#define ADC_US_SLEEP		(K * K * 1/(SAMPLE_FREQUENCY))
#define BIAS_OFFSET		(1 << ADC_BIT) >> 1

/* Bias boundary: ANALOG_VCC ± 2,5% */
#define V_MIN	((ANALOG_VCC * 0.5) - ((ANALOG_VCC / 200) * 2.5))
#define V_MAX	((ANALOG_VCC * 0.5) + ((ANALOG_VCC / 200) * 2.5))

/* Bias measure count */
#define BIAS_AVARAGE 10U

/*
 * The The GPIOs can sink or source up to ±8 mA  and sink or
 * source up to ±20 mA except PC13, PC14 and PC15 which can
 * sink or source up to ±3mA.
 */
#define MCU_MAX_CURRENT_SINK	20U /* 20mA */

#ifndef M_SQRT2
#define M_SQRT2		1.41421356237309504880
#endif

struct em_data {
	double rms_c, rms_v;
};

/*
 * -------API-------
 */

/* @ct_sensor_setup:
 */
int ct_sensor_setup(void);

/* @adc_setup:
 */
int adc_setup(void);

/* @get_measure:
 */
int get_measure(const uint8_t ch_I, const uint8_t ch_V, struct em_data *em);

/* @bias_check:
 */
int bias_check(const uint8_t ch_B);

#endif
