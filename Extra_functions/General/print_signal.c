/********************************************
 *	Koen Meessen 2013                       *
 *                                          *
 *  print_signal, print signal values       *
 *  directly from your singals in Simulink  *
 *                                          *
 ********************************************/

#define S_FUNCTION_NAME  print_signal
#define S_FUNCTION_LEVEL 2
#include "simstruc.h"
#include <math.h>

#define dmax(a,b)   ((a)>=(b) ? (a):(b))
#define dabs(a)     ((a)>= 0 ? (a):-(a))
#define DEPS        1e-15
/*================ DEFINE STATES =================*/
#define STATES          0
/*================ DEFINE GLOBAL INTEGERS =================*/
#define NIWRK           0
/*================ DEFINE GLOBAL REALS =================*/
#define NRWRK           0
/*================ DEFINE PARAMETERS =================*/
#define NPARAMS         5

#define TXTSTRING(S)        ssGetSFcnParam(S,0)
#define TIMESTAMP(S)        ssGetSFcnParam(S,1)
#define EDGEDETECT(S)       ssGetSFcnParam(S,2)
#define EDGEDETECTVAL(S)    ssGetSFcnParam(S,3)
#define DOWNSAMPLING(S)     ssGetSFcnParam(S,4)

#define TXTSTRINGMAXLEN 128


/* define global data struct */
typedef struct tag_sfun_global_data {
    int_T   counter;
    char    timestamping;
    char    edgedetection;
    double  edgedetectionvalue;
    int_T   downsampling;    
    char    txtstr[TXTSTRINGMAXLEN+1];    
    double  sampletime;
} sfun_global_data, *psfun_global_data;


static void mdlInitializeSizes(SimStruct *S)
{
    int N_INPUTS,OUTPUTPORT_SIZE=0,i;
    
    ssSetNumSFcnParams(S, NPARAMS);  
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
      return;
    
    ssSetNumContStates(S, 0); 
    ssSetNumDiscStates(S, 0); 


    if (!ssSetNumInputPorts(S,1)) return;

    ssSetInputPortWidth(S, 0, 1);
    ssSetInputPortDataType(S, 0, DYNAMICALLY_TYPED);
    ssSetInputPortDimensionInfo(S, 0, DYNAMIC_DIMENSION);
    ssSetInputPortDirectFeedThrough(S, 0, 1);
    ssSetInputPortRequiredContiguous(S, 0, 1);
    
    if (!ssSetNumOutputPorts(S,0)) 
      return;
   

    ssSetNumSampleTimes(S, 1);    
    ssSetNumRWork(S, DYNAMICALLY_SIZED);      
    ssSetNumIWork(S, sizeof(sfun_global_data)/sizeof(int_T)+1);      
    ssSetNumPWork(S, 0);          
    ssSetNumModes(S, 0);          
}


static void mdlInitializeSampleTimes(SimStruct *S)
{
     ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
     ssSetOffsetTime(S, 0, 0.0);
     
     printf("\nWARNING: SIMULINK MODEL CONTAINS PRINT_SIGNAL BLOCK, REMOVE BEFORE CHECKING IN!\n\n");
}


#define MDL_INITIALIZE_CONDITIONS


static void mdlInitializeConditions(SimStruct *S)
{
    psfun_global_data psfgd = (psfun_global_data) ssGetIWork(S);
    
    mxGetString(TXTSTRING(S), psfgd->txtstr, TXTSTRINGMAXLEN);
    psfgd->timestamping         = (int) *mxGetPr(TIMESTAMP(S));
    psfgd->edgedetection        = (int) *mxGetPr(EDGEDETECT(S));
    psfgd->edgedetectionvalue   = (double) *mxGetPr(EDGEDETECTVAL(S));
    psfgd->downsampling         = dmax((int_T) *mxGetPr(DOWNSAMPLING(S)),1);  
    psfgd->counter              = 0;
    psfgd->sampletime           = ssGetSampleTime(S,0);
}

# define MDL_SET_WORK_WIDTHS
static void mdlSetWorkWidths(SimStruct *S)
{
    int size;
    if (!ssGetInputPortConnected(S,0)) {
        size = 1;
    }else{
        size = *ssGetInputPortDimensions(S, 0);
        if(size<1){
            size = 1;
        }
    }
    ssSetNumRWork(S, size);
}

static int checkBeforePrint(psfun_global_data psfgd,real_T* u,real_T *prev_value, int_T isDouble)
{
    /* check downsampling */
    if((psfgd->counter % psfgd->downsampling) == 0){
        /* check edge_detection */
        if(psfgd->edgedetection){
            /* check current value with previous value */
            if(isDouble){
                if(dabs(*u-*prev_value) >= dmax(psfgd->edgedetectionvalue,DEPS)){
                    return 1;
                }
            }else{
                if(dabs((int)(*u)-(int)(*prev_value)) >= psfgd->edgedetectionvalue){
                    return 1;
                }                
            }
        }else{
            return 1;
        }
    }
    if(psfgd->counter == 0){
        return 1;
    }
    return 0;
}

static void printDouble(psfun_global_data psfgd,real_T *u,real_T *prev_value,real_T time_stamp)
{
    if(checkBeforePrint(psfgd, u, prev_value, 1)){
        if(psfgd->timestamping){
            printf("%.3f | %s : %f\n",time_stamp,psfgd->txtstr,*u);
        }else{
            printf("double (%s): %f\n",psfgd->txtstr,*u);
        }
    }
}

static void printInteger(psfun_global_data psfgd,real_T *u,real_T *prev_value,real_T time_stamp)
{
    if(checkBeforePrint(psfgd, u, prev_value, 0)){
        if(psfgd->timestamping){
            printf("%.3f | %s : %d\n",time_stamp,psfgd->txtstr,(int)*u);
        }else{
            printf("double (%s): %d\n",psfgd->txtstr,(int)*u);
        }
    }
}
/*=====================================================================
  ================== DEFINE OUPUTS ====================================
  =====================================================================*/

static void mdlOutputs(SimStruct *S, int_T tid)
{
    void* u;
    real_T u_val=1;
    real_T* prev_values;
    psfun_global_data psfgd;
    int_T type,size;
    real_T time_stamp;

    int i;
    /* get input value, dimension, type and global data pointers */
    prev_values = ssGetRWork(S);
    psfgd       = (psfun_global_data) ssGetIWork(S);
    u           = (void*) ssGetInputPortRealSignal(S,0);
    type        = (int)ssGetInputPortDataType(S ,0);
    size        = *ssGetInputPortDimensions(S, 0);
    
    time_stamp = ssGetT(S);
    
    for(i=0;i<size;i++){
        switch(type){
            case SS_DOUBLE :
                u_val = *((double*)u+i);
                printDouble(psfgd,&u_val,&prev_values[i],time_stamp);  
                break;
            case SS_SINGLE :
                u_val = (real_T)*((float*)u+i);
                printDouble(psfgd,&u_val,&prev_values[i],time_stamp);     
                break;
            case SS_INT8 :
            case SS_UINT8 :
                u_val = (real_T)*((char*)u+i);
                printInteger(psfgd,&u_val,&prev_values[i],time_stamp);     
                break;                
            case SS_INT16 :
            case SS_UINT16 :
                u_val = (real_T)*((short*)u+i);
                printInteger(psfgd,&u_val,&prev_values[i],time_stamp);     
                break;                   
            case SS_INT32 :
            case SS_UINT32 :
                u_val = (real_T)*((int*)u+i);
                printInteger(psfgd,&u_val,&prev_values[i],time_stamp);     
                break;                  
            case SS_BOOLEAN :
                u_val = (real_T)*((char*)u+i);
                printInteger(psfgd,&u_val,&prev_values[i],time_stamp);             
                break;
            default :
                break;
        }
        /* copy input values */
        prev_values[i] = u_val;        
    }

    psfgd->counter++;
}
static void mdlTerminate(SimStruct *S)
{ 
}

#ifdef  MATLAB_MEX_FILE
#include "simulink.c"
#else
#include "cg_sfun.h"
#endif
