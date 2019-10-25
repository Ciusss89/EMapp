#include <unistd.h>
#include <string.h>
#include <stdio.h>

/* RIOT APIs */
#include "periph/adc.h"

#define APP_NAME	"EnergMeter"

#define AD_CH_CURRENT	1U			/* CN8: A0 (PA0) Analog Input */
#define AD_CH_VOLTAGE	2U			/* CN8: A1 (PA1) Analog Input */
#define ADC_RES		ADC_RES_12BIT		/* ADC resolution */
#define ADC_SEMPLIGING	(5LU * US_PER_MS)	/* 5 ms */

/* Notes about electrical characteristics
 *  - The The GPIOs can sink or source up to ±8 mA  and sink or
 *    source up to ±20 mA except PC13, PC14 and PC15 which can
 *    sink or source up to ±3mA.
 *  - When using the PC13 to PC15 GPIOs in output mode, the
 *    speed should not exceed 2 MHz with a maximum load of 30 pF.
 *  -
 */

static int ct_sensor_setup(void)
{
	return -1;
}

static int adc_setup(void)
{
	int ret;

	if(AD_CH_CURRENT > ADC_NUMOF) {
		printf("[!] ADC channel (%u) out of range\n", AD_CH_CURRENT);
		goto err;
	} else if (AD_CH_VOLTAGE > ADC_NUMOF){
		printf("[!] ADC channel (%u) out of range\n", AD_CH_VOLTAGE);
		goto err;
	}

	ret = adc_init(ADC_LINE(AD_CH_CURRENT));
	if(ret < 0) {
		printf("[!] Initialization of ADC_LINE(%u) failed\n",
		       AD_CH_CURRENT);
		goto err;
	}

	ret = adc_init(ADC_LINE(AD_CH_VOLTAGE));
	if(ret < 0) {
		printf("[!] Initialization of ADC_LINE(%u) failed\n",
		       AD_CH_VOLTAGE);
		goto err;
	}

	return 0;
err:
	return -1;
}

int em_handler(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	int ret;

	ret = ct_sensor_setup();
	if (ret < 0)
		return -1;

	ret = adc_setup();
	if (ret < 0)
		return -1;

	printf("Welcome to %s\n", APP_NAME);

	return 0;
}
