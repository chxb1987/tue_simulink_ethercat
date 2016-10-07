/*
        ec_TU0001_combi

        (c) Koen Meessen, 2009

        Outputs:y[0]  = status
		y[1]  = counter
		y[2]  = diginput
		y[3]  = analog[0]
		      |
		y[10] = analog[7]
		y[11] = CAM1status
		y[12] = CAM1posX
		y[13] = CAM1posY
		y[14] = CAM1posZ
		y[15] = CAM1velX
		y[16] = CAM1velY
		y[17] = CAM1velZ
		y[18] = CAM1timestamp
		y[19] = SHTstatus
		y[20] = SHTvalue
                y[21] = AccelerationX
                y[22] = AccelerationY
                y[23] = AccelerationZ

	Inputs: u[0]  = CAM1control
		u[1]  = CAM1data
		u[2]  = SHTcontrol
		u[3]  = SHTdutycycle
		u[4]  = SHTpulselength
		u[5]  = SHTencodermax
					

        Parameter: link id
*/
#define S_FUNCTION_NAME ec_TU0001_combi
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#define NSTATES         0
#define NINPUTS         6
#define NOUTPUTS        24
#define NPARAMS         1

#define LINK_ID         ssGetSFcnParam(S,0)

#define U(element) (*uPtrs[element])  /* Pointer to Input Port0 */

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

	if (!ssSetNumInputPorts(S,1)) return;
	ssSetInputPortWidth(S,0,NINPUTS);
	ssSetInputPortDirectFeedThrough(S,0,0); 

	if (!ssSetNumOutputPorts(S,1)) return;
	ssSetOutputPortWidth(S,0,NOUTPUTS);

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
        InputRealPtrsType uPtrs=ssGetInputPortRealSignalPtrs(S,0);      
#ifndef MATLAB_MEX_FILE
        int_T ilink, ireadchan, iwritechan;

        ilink=(int_T) (*(mxGetPr(LINK_ID)));
/*      read channel */
        for (ireadchan=0; ireadchan<NOUTPUTS; ireadchan++) {
        	ec_TU0001_combi_read_chan(&y[ireadchan], ireadchan, ilink);
	}

/*      write channel */

        for (iwritechan=0; iwritechan<NINPUTS; iwritechan++) {
            ec_TU0001_combi_write_chan(U(iwritechan), iwritechan, ilink);
        }
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
