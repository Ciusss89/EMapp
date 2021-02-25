/**
 * Author:	Giuseppe Tipaldi
 * Created:	2019
 **/

#ifndef CT_H
#define CT_H

/* Current Transformers (CT) sensor YHDC TA1020 */
/*#define CT_MAX_INPUT	20U Assume a range  0-20A max */
/*#define CT_MAX_OUPUT	20U Max rated ouput in mA */
/*#define CT_RATIO	1000U	Turns ratio */
/*#define CT_LOAD		50U	Max Sampling resistance in Ω */
/*#define CT_PHASE_SHIFT	12U	Phase shitf 12.3' */

/* Current Transformers (CT) sensor YHDC SCT013-030 */
#define CT_MAX_INPUT	30U	/* Assume a range  0-30A max */
#define CT_RATIO	1800U	/* Turns ratio */
#define CT_LOAD		50U	/* Max Sampling resistance in Ω */
#define CT_PHASE_SHIFT	12U	/* Phase shitf 12.3' */

#endif
