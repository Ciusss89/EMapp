#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "xtimer.h"
#include "timex.h"
#include "periph/adc.h"

/*
 * The ADC is working on the all A0-A5 analog input lines of CN8
 *  1) RIOT/boards/nucleo-f401re/include/periph_conf.h
 *  2) RIOT/cpu/stm32f4/periph/adc.c
 *
 *  FSCR -> 0 to 3.3V
 */

#define RES             ADC_RES_12BIT
#define DELAY           (1000LU * US_PER_MS) /* 1000 ms */

int main(void)
{
	xtimer_ticks32_t last = xtimer_now();
	int sample = 0;

	printf("RIOT on a %s board, MCU %s\n\r", RIOT_BOARD, RIOT_MCU);

    	printf("Print all available ADC lines once every 100ms to STDIO\n");

	/* initialize all available ADC lines */
	for (unsigned i = 0; i < ADC_NUMOF; i++) {
		if (adc_init(ADC_LINE(i)) < 0) {
	            printf("Initialization of ADC_LINE(%u) failed\n", i);
	           goto err;
		} else {
			printf("Successfully initialized ADC_LINE(%u)\n", i);
		}
	}

	while (1) {
	        for (unsigned i = 0; i < ADC_NUMOF; i++) {
        		sample = adc_sample(ADC_LINE(i), RES);
		            if (sample < 0) {
				    printf("ADC_LINE(%u): 10-bit resolution not applicable\n", i);
			    } else {
				    printf("ADC_LINE(%u): %i\n", i, sample);
		            }
	        }
	        xtimer_periodic_wakeup(&last, DELAY);
	}

	return 0;

err:
	return 1;
}
