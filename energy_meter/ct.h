/**
 * Author:	Giuseppe Tipaldi
 * Created:	2019
 **/

#ifndef CT_H
#define CT_H

/* Current Transformers (CT) sensor YHDC TA1020 */
#if CT_TYPE == 0
#define CT_MAX_INPUT	20U /* Assume a range  0-20A max */
#define CT_MAX_OUPUT	20U /* Max rated ouput in mA */
#define CT_RATIO	1000U /* Turns ratio */
#define CT_LOAD		50U /* Max Sampling resistance in Ω */
#define CT_PHASE_SHIFT	12U /* Phase shitf 12.3' */
#elif CT_TYPE == 1
/* Current Transformers (CT) sensor YHDC SCT-013-000*/
#define CT_MAX_INPUT 100 /* Max rated input 100A */
#define CT_RATIO 2000 /* Turns ratio (100A:50mA) */
#else
#error "SELECT THE CURRENT TRANSFORMER"
#endif

#endif
