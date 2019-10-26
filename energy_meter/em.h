
#ifndef EM_H
#define EM_H

#include <stdlib.h>
#include "ct102.h"

#define MAX_CURRENT	12U	/* Maxinum Measurable Current */
#define K		1000

/*
 * I prefer don't have a rail-to-rail waveform, such expedient should
 * reduce the no-linearity contribs. I instead to set 3.3V I'll use 3V
 */
#define ANALOG_IN_VPP	3U

#ifndef M_SQRT2
#define M_SQRT2		1.41421356237309504880
#endif

int em_handler(int argc, char *argv[]);

#endif
