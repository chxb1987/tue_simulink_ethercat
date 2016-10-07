/*
 * ec_TU_ES_030v2FRF
 *
 * (c)  Alex Andrien, 2016
 *
 * Outputs (to slave):
 * Actuator intputs
 * u[0] = mcom
 * u[0_0] = motor_select1
 * u[0_1] = motor_select2
 * u[0_2] = enable_motor
 * u[0_3] = tristate_motor
 * u[0_4] = unused
 * u[0_5] = unused
 * u[0_6] = unused
 * u[0_7] = unused
 * u[1] = go
 *
 * Inputs (from slave):
 * Sensor outputs
 * y[0] = mstate
 * y[1] = buffer
 * y[2] = entries
 * y[3] = ectime
 * y[4] = current[0]
 * |
 * y[25] = current[21]
 *
 *
 * Parameters:
 * p[0] = link_id
 * p[1] = slave_num
 * p[2] = [m1r, m2r, m3r]
 * p[3] = [m1kv, m2kv, m3kv]
 * p[4] = [m1pgain, m2pgain, m2pgain]
 * p[5] = [m1igain, m2igain, m3igain]
 * p[6] = [m1ilimit, m2ilimit, m3ilimit]
 * p[7] = [m1encdir, m2encdir, m3encdir]
 * p[8] = [m1encres, m2encres, m3encres]
 * p[9] = [m1czero, m2czero, m3czero]
 * p[10] = setpoints
 * p[11] = numsets
 */

#define S_FUNCTION_NAME ec_TU_ES_030FRF
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#define NSTATES         0
#define NINPUTS         2
#define NOUTPUTS        26
#define NPARAMS         12

#define LINK_ID         ssGetSFcnParam(S,0)
#define SLAVE_NUM       ssGetSFcnParam(S,1)
#define SETPOINTS       ssGetSFcnParam(S,10)
#define NSETPOINTS      ssGetSFcnParam(S,11)

#define U(element) (*uPtrs[element])  /* Pointer to Input Port0 */

#include <math.h>
#include "ec.h"

int done = 0;
int entries = 0;
int entries_o_tot = 0;
int est_t = 0;
int firstrun = 0;
int once = 1;
int nsets = 0;
int setcount = 0;
int setleft = 0;
int time_once = 0;

typedef struct {
    int *isets;
} SfunctionGlobalData, *pSfunctionGlobalData;

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
    ssSetNumRWork(S, sizeof(SfunctionGlobalData)/sizeof(real_T)+1);
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
    pSfunctionGlobalData psfgd = (pSfunctionGlobalData) ssGetRWork(S);
    
    real_T *y=ssGetOutputPortRealSignal(S,0);
    InputRealPtrsType uPtrs=ssGetInputPortRealSignalPtrs(S,0);
    
#ifndef MATLAB_MEX_FILE
    
    int_T ilink, slavenum, ireadchan, iwritechan;
    
    ilink= (int_T) (*(mxGetPr(LINK_ID)));
    slavenum= (int_T) (*(mxGetPr(SLAVE_NUM)));
    
    /*      Write Parameters on first run*/
    if (firstrun == 0) {
        if(ec_Set_TUeES030mode(2,slavenum) != 0) {
            printf("An error ocurred during switching mode of slave %d \n",slavenum);
        }
        int iwriteparam;
        for (iwriteparam=2; iwriteparam<10; iwriteparam++) {
            double *params = (double *) (mxGetPr(ssGetSFcnParam(S,iwriteparam)));
            ec_Set_TUeES030params(params, (iwriteparam-1), slavenum);
        }
        double *sets = (double *) (mxGetPr(SETPOINTS)); //Retrieve array of setpoints
        nsets = (*(mxGetPr(NSETPOINTS))); //Retrieve total number of setpoints
        est_t = nsets/20000;
        psfgd->isets = malloc(nsets*sizeof(int));
        int ii;
        for (ii = 0; ii<nsets; ii++) {
            psfgd->isets[ii] = (int) sets[ii];
        }
        setleft = nsets;
        printf("Number of setpoints: %d \n",nsets);
        firstrun = 1;
    }
    int go = U(1); //Retrieve go signal
    
    /*      read channel */
    for (ireadchan=0; ireadchan<NOUTPUTS; ireadchan++) {
        ec_TU_ES_030FRF_read_chan(&y[ireadchan], ireadchan, ilink);
    }
    
    /*      Retrieve number of useful current values */
    int entries_o = (int) (y[2]);
      
    /*      Calculate number of entries to motor depending on buffer fill */
    int buf = (y[1]);
    if (go) {
        if(time_once == 0) {
            printf("Measurement started, estimated time (@20kHz) = %d [s]\n",est_t);
            time_once = 1;
        }
        if (buf < 50) {
            entries = 22;
        }
        else if (buf == 50) {
            entries = 20;
        }
        else if (buf > 50) {
            entries = 18;
        }
        if (setleft<entries) {
            entries = setleft;
        }
    }
    else {
        entries = 0;
    }
    
    /*      write channel */
    for (iwritechan=0; iwritechan<24; iwritechan++) {
        if (iwritechan == 0) {
            ec_TU_ES_030FRF_write_chan(U(iwritechan), iwritechan, ilink); //write mode
        }
        else if (iwritechan == 1) {
            ec_TU_ES_030FRF_write_chan(entries, iwritechan, ilink); //write entries
        }
        else if (((iwritechan-2) < entries) && (go) && (!done)){
            ec_TU_ES_030FRF_write_chan(psfgd->isets[iwritechan-2+setcount], iwritechan, ilink); 
            //write setpoints
        }
        else {
            ec_TU_ES_030FRF_write_chan(0, iwritechan, ilink); //write 0 after setpoints
        }
    }
    
    entries_o_tot = entries_o_tot+entries_o;
    
    if (setcount >= nsets) {
        done = 1;
    }
    setcount = setcount + entries;
    setleft = nsets-setcount;
    
    if((entries_o_tot >= nsets) && (once)){
        printf("Done!\n");
        printf("Current count = %d \n",entries_o_tot);
        printf("Setcount = %d \n",setcount);
        once = 0;
    }
#endif
}

static void mdlTerminate(SimStruct *S)
{
    pSfunctionGlobalData psfgd = (pSfunctionGlobalData) ssGetRWork(S);
    
    free(psfgd->isets);
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
