/*
        ec_EL5101_enc

        Koen Meessen 2014

        Outputs: y[0] = enc channel 1
		Outputs: y[1] = enc channel 2

        Parameter: link id
*/

#define S_FUNCTION_NAME ec_EL5152_enc
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#define NSTATES         0
#define NINPUTS         0
#define NOUTPUTS        2
#define NPARAMS         1

#define LINK_ID         ssGetSFcnParam(S,0)

#include <math.h>
#include "ec.h"

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
	ssSetOutputPortWidth(S,0,1);
	ssSetOutputPortWidth(S,1,1);

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
	real_T *y[2];
	y[0] = ssGetOutputPortRealSignal(S,0);
    y[1] = ssGetOutputPortRealSignal(S,1);
#ifndef MATLAB_MEX_FILE
    int_T ilink;

    ilink=(int_T) (*(mxGetPr(LINK_ID)));

/*  read enc channel */
    ec_EL5152_enc_read_chan(y[0], ilink);
#endif
}

static void mdlTerminate(SimStruct *S)
{
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
