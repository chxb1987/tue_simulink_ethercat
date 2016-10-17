/*
 *
 *		sf_setcpu
 *
 *		Simulink S-function to set cpu affinity of the model
 *
 *		Rene van de Molengraft, April, 10th, 2009
 *
 */

#include "sf_setcpu.h"

static void mdlInitializeSizes(SimStruct *S)
{
	int_T Rworksize;

        ssSetNumSFcnParams(S,NPARAMS);
        if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
            return; /* Parameter mismatch will be reported by Simulink */
        }
    
        if (!ssSetNumInputPorts(S, NINPORTS)) return;
    
        if (!ssSetNumOutputPorts(S, NOUTPORTS)) return;
        ssSetOutputPortWidth(S, 0, NOUTPUTS);

        ssSetNumSampleTimes(S, 1);
    
/*      compute necessary amount of real_T workspace */
	Rworksize=( sizeof(sfun_global_data)/sizeof(real_T) + 1 );
	//print("sf_setcpu reports: Rworksize = %d\n",Rworksize);
	ssSetNumRWork(S,Rworksize);
}





static void mdlInitializeSampleTimes(SimStruct *S)
{
        ssSetSampleTime(S, 0, 0.0);
        ssSetOffsetTime(S, 0, 0.0);
}





#define MDL_INITIALIZE_CONDITIONS
#if defined(MDL_INITIALIZE_CONDITIONS)
static void mdlInitializeConditions(SimStruct *S)
{
    	real_T* sfpar_cpu_id=(real_T*) mxGetPr(SFPAR_CPU_ID(S));
	real_T* ptrRwrk=ssGetRWork(S);
	psfun_global_data psfgd;

/*	get pointer to S-function global data */
	psfgd=(psfun_global_data) ptrRwrk;
        psfgd->Ncores = (int)sysconf(_SC_NPROCESSORS_ONLN);
        if(psfgd->Ncores < 1){
            psfgd->Ncores = 1;
        }
        psfgd->first_time=1;
        psfgd->cpu_id=((int_T) *sfpar_cpu_id)-1; /* cpu_id counts from 0... */
}
#endif





static void mdlOutputs(SimStruct *S, int_T tid)
{
        real_T *y=(real_T*) ssGetOutputPortRealSignal(S, 0);
	real_T* ptrRwrk=ssGetRWork(S);
	psfun_global_data psfgd;
        
/*	get pointer to S-function global data */
	psfgd=(psfun_global_data) ptrRwrk;

        if (psfgd->first_time) {
/*		set cpu affinity mask */
/*		do this in output function, because this function is not used during the RTW process! */
		CPU_ZERO(&(psfgd->mask));      
#ifdef SIMULATOR
        /* Simulator mode, hence, set cpu_id alternating based on AGENT number */
        /* overwrite current id */
        psfgd->cpu_id = (int_T) (Whoami() % psfgd->Ncores);
#endif
		CPU_SET(psfgd->cpu_id, &(psfgd->mask));
		if (sched_setaffinity(0, sizeof(cpu_set_t), &(psfgd->mask))==-1) {
			print("sf_setcpu reports: could not set cpu affinity.\n");
		} else {
			print("sf_setcpu reports: affinity set to cpu %d.\n", psfgd->cpu_id);
		}
		psfgd->first_time=0;
        }

        y[0]=0.0;
}





static void mdlTerminate(SimStruct *S)
{
	return;
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif

