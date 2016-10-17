/*
 * S-Function to Interface with EtherCAT hardware
 *
 *      Created on: 07-2015
 *      Author: FSCHOENM
 *
 */

#define S_FUNCTION_NAME ECAT_Interface
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include <math.h>
#include "ec.h"

#define NSTATES		0
#define NINPUTS		0
#define NOUTPUTS	1
#define NPARAMS		2

/* define global data struct */
typedef struct tag_sfun_global_data {
    char eth_interface[100];
} sfun_global_data, *psfun_global_data;

/*====================*
 * S-function methods *
 *====================*/

#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
static void mdlCheckParameters(SimStruct *S)
{
}
#endif /* MDL_CHECK_PARAMETERS */

static void mdlInitializeSizes(SimStruct *S)
{
    int_T Rworksize;
	ssSetNumSFcnParams(S,NPARAMS);  /* Number of expected parameters */
#if defined(MATLAB_MEX_FILE)
	if (ssGetNumSFcnParams(S)==ssGetSFcnParamsCount(S)) {
	  mdlCheckParameters(S);
	  if (ssGetErrorStatus(S)!=NULL) {
	    return;
	  }
	} else {
	  return; /* Parameter mismatch will be reported by Simulink */
	}
#endif

	ssSetNumContStates(S,NSTATES);
	ssSetNumDiscStates(S,0);

	/***************************
     * Input Ports definitions *
     ***************************/
    if (!ssSetNumInputPorts(S,NINPUTS)) return;    

    /****************************
     * Output Ports definitions *
     ****************************/
	if (!ssSetNumOutputPorts(S,NOUTPUTS)) return;
    ssSetOutputPortWidth(S,0,NOUTPUTS);
    ssSetOutputPortDataType(S,0,SS_INT32);
    
	/**********************
     * Default Definitions *
     **********************/
	ssSetNumSampleTimes(S,1);
    /* compute necessary amount of real_T workspace */
    Rworksize = ( sizeof(sfun_global_data)/sizeof(real_T) + 1 );
	ssSetNumRWork(S,Rworksize);
	ssSetNumIWork(S,0);
	ssSetNumPWork(S,0);
	ssSetNumModes(S,0);
	ssSetNumNonsampledZCs(S,0);
    
    /* set block to execute first in the Simulink hierarchy */
    ssSetOptions(S, SS_OPTION_PLACE_ASAP );
}

static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, *mxGetPr(ssGetSFcnParam(S,1)));
    ssSetOffsetTime(S, 0, 0.0);
}

#define MDL_START /* Change to #undef to remove function */
#if defined(MDL_START)
/* Function: mdlStart =======================================================
* Abstract:
* This function is called once at start of model execution. If you
* have states that should be initialized once, this is the place
* to do it.
*/
static void mdlStart(SimStruct *S)
{
#ifndef MATLAB_MEX_FILE    
	/* get pointers to global data */
    int_T buflen;
    char String[100];
    real_T* ptrRwrk = ssGetRWork(S);
    psfun_global_data psfgd;
    psfgd = (psfun_global_data) ptrRwrk;
    /* Init rework memory */
    /* Get length of device string */
    buflen = (int_T)mxGetN(ssGetSFcnParam(S, 0))*sizeof(mxChar)+1; 
    /* Get String from parameter */
    mxGetString(ssGetSFcnParam(S, 0), String, buflen);
    /* Store string in rwork memory */
    sprintf(psfgd->eth_interface,"%s",String); 
    
    ssPrintf("Using SOEM\n");
    ssPrintf("Net interface = %s\n", psfgd->eth_interface);

    if (ec_initial(psfgd->eth_interface) != 0){
        ssPrintf("Initialization failed!\n");
        ssSetErrorStatus(S, "EtherCAT initialization failed!");
        return;
    }else{
        ssPrintf("Initialization succeeded\n");
    }
    if (ec_scan() != 0){
        ssPrintf("setting to OP state failed!\n");
        ssSetErrorStatus(S, "Setting devices to OP state failed!");
        return;
    }else{
        ssPrintf("setting to OP state succeeded\n");
    }
#endif /* MATLAB_MEX_FILE */    
}
#endif /* MDL_START */

static void mdlOutputs(SimStruct *S, int_T tid)
{
    int* ECAT_state;
    /* Pointer to output port */
	ECAT_state = (int*) ssGetOutputPortRealSignal(S,0);
#ifndef MATLAB_MEX_FILE     
    ECAT_state[0] = ec_getstate();

	if(ec_io()){
        ssPrintf("ec_io() NOT succeeded!\n");
    }
#else
    ECAT_state[0] = 0;
#endif /* MATLAB_MEX_FILE */    
}

static void mdlUpdate(SimStruct *S, int_T tid)
{
}

static void mdlTerminate(SimStruct *S)
{   
#ifndef MATLAB_MEX_FILE    
    ec_exit();
#endif /* MATLAB_MEX_FILE */    
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
