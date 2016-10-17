/*
        ec_TU_ES_030

        (c) Ferry Schoenmakers, 2015

Inputs:
    Digital_inputs
            y[0_0] = enable_1
            y[0_1] = enable_2
            y[0_2] = spare_do_3
            y[0_3] = spare_do_4
            y[0_4] = reserved_1
            y[0_5] = reserved_2
            y[0_6] = reserved_3
            y[0_7] = reserved_4        
        y[0] = port (union)
    Actuator intputs
        y[1] = m_1
        y[2] = m_2
        y[3] = m_3
        y[4] = aout_1
        y[5] = aout_2

Outputs: 
        Digital_outputs:
            u[0_0] = spare_di_1
            u[0_1] = spare_di_2
            u[0_2] = spare_di_3
            u[0_3] = spare_di_4
            u[0_4] = reserved_1
            u[0_5] = reserved_2
            u[0_6] = reserved_3
            u[0_7] = powerstatus        
        u[0] = port (union)
    Sensor outputs
        u[1] = encoder_1
        u[2] = encoder_2
        u[3] = encoder_3
        u[4] = current_1
        u[5] = current_2
        u[6] = current_3
        u[7] = caliper_1
        u[8] = caliper_2  
        u[9] = force_1
        u[10] = force_2
        u[11] = force_3
        u[12] = pos_1
        u[13] = pos_2
        u[14] = pos_3
        u[15] = spare_ai_1
        u[16] = spare_ai_2
        u[17] = time_stamp
					

        Parameter: link id
*/
#define S_FUNCTION_NAME ec_TU_ES_030old
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#define NSTATES         0
#define NINPUTS         6
#define NOUTPUTS        18
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
        	ec_TU_ES_030old_read_chan(&y[ireadchan], ireadchan, ilink);
	}

/*      write channel */

        for (iwritechan=0; iwritechan<NINPUTS; iwritechan++) {
            ec_TU_ES_030old_write_chan(U(iwritechan), iwritechan, ilink);
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
