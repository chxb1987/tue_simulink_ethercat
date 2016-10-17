/*
        ec_TU_ES_030v3

        (c) Ferry Schoenmakers, 2015, Alex Andrien, 2016

Outputs (to slave):
        u[1] = mcom1
        u[2] = setpoint1
        u[3] = ff1
        u[4] = mcom2
        u[5] = setpoint2
        u[6] = ff2
        u[7] = mcom3
        u[8] = setpoint3
        u[9] = ff3
        u[10] = digital_o
            Digital_outputs: (to slave)
            u[10_0] = enable_1
            u[10_1] = enable_2
            u[10_2] = spare_do_3
            u1[0_3] = spare_do_4
            u[10_4] = reserved_1
            u[10_5] = reserved_2
            u[10_6] = reserved_3
            u[10_7] = reserved_4        
        u[11] = aout1
        u[12] = aout2

Inputs (from slave): 
        y[0] = mstate1
        y[1] = count1
        y[2] = timestamp1
        y[3] = velocity1
        y[4] = current1
        y[5] = mstate2
        y[6] = count2
        y[7] = timestamp2
        y[8] = velocity2
        y[9] = current2
        y[10] = mstate3
        y[11] = count3
        y[12] = timestamp3
        y[13] = velocity3
        y[14] = current3
        y[15] = digital_i
            Digital_inputs: (from slave)
            y[15_0] = spare_di_1
            y[15_1] = spare_di_2
            y[15_2] = spare_di_3
            y[15_3] = spare_di_4
            y[15_4] = reserved_1
            y[15_5] = reserved_2
            y[15_6] = reserved_3
            y[15_7] = powerstatus
        y[16] = caliper1
        y[17] = caliper2  
        y[18] = force1
        y[19] = force2
        y[20] = force3
        y[21] = pos1
        y[22] = pos2
        y[23] = pos3
        y[24] = analog1
        y[25] = analog2
        y[26] = linevoltage
        y[27] = ectime
					

Parameters: 
        p[0] = link_id
        p[1] = slave_num
        p[2] = [m1r, m2r, m3r]
        p[3] = [m1kv, m2kv, m3kv]
        p[4] = [m1pgain, m2pgain, m2pgain]
        p[5] = [m1igain, m2igain, m3igain]
        p[6] = [m1ilimit, m2ilimit, m3ilimit]
        p[7] = [m1encdir, m2encdir, m3encdir]
        p[8] = [m1encres, m2encres, m3encres]
        p[9] = [m1czero, m2czero, m3czero]
*/
#define S_FUNCTION_NAME ec_TU_ES_030
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#define NSTATES         0
#define NINPUTS         12
#define NOUTPUTS        28
#define NPARAMS         10

#define LINK_ID         ssGetSFcnParam(S,0)
#define SLAVE_NUM       ssGetSFcnParam(S,1)

#define U(element) (*uPtrs[element])  /* Pointer to Input Port0 */

#include <math.h>
#include "ec.h"

int firstrun[6] = {0};
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
    
    int_T ilink, slavenum, ireadchan, iwritechan;
    
    ilink= (int_T) (*(mxGetPr(LINK_ID)));
    slavenum= (int_T) (*(mxGetPr(SLAVE_NUM)));
    
    /*      Write Parameters on first run*/
    if (firstrun[ilink] == 0) {
//         if(ec_Set_TUeES030mode(1,slavenum) != 0) {
//             printf("An error ocurred during switching mode of slave %d \n",slavenum);
//         }
        int iwriteparam;
        for (iwriteparam=2; iwriteparam<NPARAMS; iwriteparam++) {
            double *params = (double *) (mxGetPr(ssGetSFcnParam(S,iwriteparam)));
            ec_Set_TUeES030params(params, (iwriteparam-1), slavenum);
        }
        firstrun[ilink] = 1;
    }
    
    /*      read channels */
    for (ireadchan=0; ireadchan<NOUTPUTS; ireadchan++) {
        ec_TU_ES_030_read_chan(&y[ireadchan], ireadchan, ilink);
    }
    
    /*      write channels */
    for (iwritechan=0; iwritechan<NINPUTS; iwritechan++) {
        ec_TU_ES_030_write_chan(U(iwritechan), iwritechan, ilink);
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
