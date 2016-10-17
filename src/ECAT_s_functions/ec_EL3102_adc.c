/*
        ec_EL3102_adc

        (c) Rene' van de Molengraft, 2007

        Outputs: y[0] - y[1]  = adc channels 0, 1

        Parameter: link id
*/

#define S_FUNCTION_NAME ec_EL3102_adc
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include <math.h>
#include "ec.h"

#define NSTATES         0
#define NINPUTS         0
#define NOUTPUTS        1
#define NOUTPUTS0       2
#define NPARAMS         1

#define LINK_ID         ssGetSFcnParam(S,0)

/*====================*
 * S-function methods *
 *====================*/

static void mdlInitializeSizes(SimStruct *S)
{
	ssSetNumSFcnParams(S,NPARAMS);

	ssSetNumContStates(S,NSTATES);
	ssSetNumDiscStates(S,0);

	if (!ssSetNumInputPorts(S,0)) return;

	if (!ssSetNumOutputPorts(S,NOUTPUTS)) return;
	ssSetOutputPortWidth(S,0,NOUTPUTS0);
    ssSetOutputPortDataType(S,0,SS_DOUBLE);

	ssSetNumSampleTimes(S,1);
	ssSetNumRWork(S,0);
	ssSetNumIWork(S,0);
	ssSetNumPWork(S,0);
	ssSetNumModes(S,0);
	ssSetNumNonsampledZCs(S,0);
}





static void mdlInitializeSampleTimes(SimStruct *S)
{
	ssSetSampleTime(S,0,CONTINUOUS_SAMPLE_TIME);
	ssSetOffsetTime(S, 0, FIXED_IN_MINOR_STEP_OFFSET);
}





static void mdlOutputs(SimStruct *S, int_T tid)
{
        real_T *y=ssGetOutputPortRealSignal(S,0);

        int_T ichan, ilink;

        ilink=(int_T) (*(mxGetPr(LINK_ID)));

        for (ichan=0; ichan<NOUTPUTS0; ichan++) {
/*          read adc channels */
            ec_EL3102_adc_read_chan(&y[ichan], ichan, ilink);
        }
}





static void mdlTerminate(SimStruct *S)
{
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
