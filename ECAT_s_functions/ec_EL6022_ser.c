/*
        ec_EL6022_ser s-function for RS485 ethercat slave

        Written by: Koen Meessen (March 2011)
        Edited: Geert-Jan Heldens (April 2011)
 
        Parameter: link id
*/

#define S_FUNCTION_NAME ec_EL6022_ser
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "stdio.h"
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "math.h"
#include <string.h>

#define NSTATES         0
#define NINPUTS         4
#define NINPUTS0        22 /* data channel 0 */
#define NINPUTS1        1  /* n bytes channel 0 */
#define NINPUTS2        22 /* data channel 1 */
#define NINPUTS3        1  /* n bytes channel 1 */

#define NOUTPUTS        6 
#define NOUTPUTS0       22 /* data channel 0 */
#define NOUTPUTS1       1  /* n bytes channel 0 */
#define NOUTPUTS2       1  /* Tx flag channel 0 */
#define NOUTPUTS3       22 /* data channel 1 */
#define NOUTPUTS4       1  /* n bytes channel 1 */
#define NOUTPUTS5       1  /* Tx flag channel 1 */

#define NPARAMS         1

#define LINK_ID         ssGetSFcnParam(S,0)

#define dmax(a,b)   ((a)>=(b) ? (a):(b))
#define dmin(a,b)   ((a)<=(b) ? (a):(b))

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef bool boolean;

#ifndef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "Rs485EthercatLibrary.h"
#include "ec.h"
#endif	

/*====================*
 * S-function methods *
 *====================*/
/* define global data struct */
typedef struct tag_sfun_global_data {
    int_T RS485Index;
    uint16 NumberOfRS485;
    time_t LastPrintTime;
    int_T FirstTime;
} sfun_global_data, *psfun_global_data;

static void mdlInitializeSizes(SimStruct *S)
{
	ssSetNumSFcnParams(S,NPARAMS);

	ssSetNumContStates(S,NSTATES);
	ssSetNumDiscStates(S,0);

	if (!ssSetNumInputPorts(S,NINPUTS)) return;
    
    ssSetInputPortWidth(S, 0, NINPUTS0);
    ssSetInputPortDataType(S, 0, SS_INT8);
    ssSetInputPortDirectFeedThrough(S, 0, 1);
    ssSetInputPortRequiredContiguous(S, 0, 1);
    
    ssSetInputPortWidth(S, 1, NINPUTS1);
    ssSetInputPortDataType(S, 1, SS_INT8);
    ssSetInputPortDirectFeedThrough(S, 1, 1);
    ssSetInputPortRequiredContiguous(S, 1, 1);
    
    ssSetInputPortWidth(S, 2, NINPUTS2);
    ssSetInputPortDataType(S, 2, SS_INT8);
    ssSetInputPortDirectFeedThrough(S, 2, 1);
    ssSetInputPortRequiredContiguous(S, 2, 1);
    
    ssSetInputPortWidth(S, 3, NINPUTS3);
    ssSetInputPortDataType(S, 3, SS_INT8);
    ssSetInputPortDirectFeedThrough(S, 3, 1);
    ssSetInputPortRequiredContiguous(S, 3, 1);

	if (!ssSetNumOutputPorts(S,NOUTPUTS)) return;
    ssSetOutputPortWidth(S, 0, NOUTPUTS0);
    ssSetOutputPortDataType(S, 0, SS_INT8);

    ssSetOutputPortWidth(S, 1, NOUTPUTS1);
    ssSetOutputPortDataType(S, 1, SS_INT8);
    
    ssSetOutputPortWidth(S, 2, NOUTPUTS2);
    ssSetOutputPortDataType(S, 2, SS_INT8); 
    

    ssSetOutputPortWidth(S, 3, NOUTPUTS3);
    ssSetOutputPortDataType(S, 3, SS_INT8);

    ssSetOutputPortWidth(S, 4, NOUTPUTS4);
    ssSetOutputPortDataType(S, 4, SS_INT8);
   
    ssSetOutputPortWidth(S, 5, NOUTPUTS5);
    ssSetOutputPortDataType(S, 5, SS_INT8);
    
	ssSetNumSampleTimes(S,1);

    ssSetNumRWork(S, sizeof(sfun_global_data)/sizeof(real_T)+1);      
	ssSetNumIWork(S,0);
	ssSetNumPWork(S,0);
	ssSetNumModes(S,0);
	ssSetNumNonsampledZCs(S,0);
}

static void mdlInitializeSampleTimes(SimStruct *S)
{
	ssSetSampleTime(S,0,0.0);
	ssSetOffsetTime(S, 0, FIXED_IN_MINOR_STEP_OFFSET);
}

#define MDL_INITIALIZE_CONDITIONS

static void mdlInitializeConditions(SimStruct *S)
{
    psfun_global_data psfgd = (psfun_global_data) ssGetRWork(S);
    
    psfgd->RS485Index = (int_T) (*(mxGetPr(LINK_ID)))-1;
    psfgd->LastPrintTime = time(NULL);
    psfgd->FirstTime = 1;
}

static void mdlOutputs(SimStruct *S, int_T tid)
{
    /* Input ports */
    uint8_T *DataOutCh0    = (uint8_T*) ssGetInputPortRealSignal(S,0); 
    uint8_T DataOutSizeCh0 = *(uint8_T*)ssGetInputPortRealSignal(S,1); 
    uint8_T *DataOutCh1    = (uint8_T*) ssGetInputPortRealSignal(S,2);   
    uint8_T DataOutSizeCh1 = *(uint8_T*)ssGetInputPortRealSignal(S,3);    

    /* Output ports */
    uint8_T *DataInCh0     = (uint8_T*)ssGetOutputPortSignal(S,0); 
    uint16  *DataInSizeCh0 = (uint16*) ssGetOutputPortSignal(S,1); 
    int8_T  *Flag_Tx0      = (int8_T*) ssGetOutputPortSignal(S,2);     
    uint8_T *DataInCh1     = (uint8_T*)ssGetOutputPortSignal(S,3);   
    uint16  *DataInSizeCh1 = (uint16*) ssGetOutputPortSignal(S,4);     
    int8_T  *Flag_Tx1      = (int8_T*) ssGetOutputPortSignal(S,5);       
    
    psfun_global_data psfgd = (psfun_global_data) ssGetRWork(S);
#ifndef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */        
    
    /* Define variables */
    int_T i;
    uint16 BytesWritten=0, BytesRead=0;
    uint8_T data_in[RS485_MAX_DATA_LENGTH];
    boolean FlagTxCh0 = 0, FlagTxCh1 = 0;        
    
    uint16 NumberOfRS485=0;
    RS485_SETTINGS settings_ch1, settings_ch2;

    /* process data from ethercat slave */
    RS485EcProcessStateMachine(psfgd->RS485Index);

    if(psfgd->FirstTime){
        /* intialize */
        RS485EcGetNumberOfDevices(&NumberOfRS485);
        if(NumberOfRS485 < 1){
        	if(psfgd->LastPrintTime + 1.0 < time(NULL)){
                printf("No RS485 Devices found\n");
            	psfgd->LastPrintTime = time(NULL);
        	}
            return;
        }

// Old settings for switched VPU and DXL
//         settings_ch1.Baudrate			= RS485_115200_BAUD;
//         settings_ch1.DataFrame			= RS485_8B_NP_1S;
//         settings_ch1.Duplex				= RS485_HALF_DUPLEX;
//         settings_ch1.SendHandshake		= XON_XOFF_DISABLE;
//         settings_ch1.ReceiveHandshake	= XON_XOFF_DISABLE;
// 
// 
//         settings_ch2.Baudrate			= RS485_19200_BAUD;
//         settings_ch2.DataFrame			= RS485_8B_NP_1S;
//         settings_ch2.Duplex				= RS485_FULL_DUPLEX;
//         settings_ch2.SendHandshake		= XON_XOFF_DISABLE;
//         settings_ch2.ReceiveHandshake	= XON_XOFF_DISABLE;
        
        settings_ch2.Baudrate			= RS485_115200_BAUD;
        settings_ch2.DataFrame			= RS485_8B_NP_1S;
        settings_ch2.Duplex				= RS485_HALF_DUPLEX;
        settings_ch2.SendHandshake		= XON_XOFF_DISABLE;
        settings_ch2.ReceiveHandshake	= XON_XOFF_DISABLE;


        settings_ch1.Baudrate			= RS485_19200_BAUD;
        settings_ch1.DataFrame			= RS485_8B_NP_1S;
        settings_ch1.Duplex				= RS485_FULL_DUPLEX;
        settings_ch1.SendHandshake		= XON_XOFF_DISABLE;
        settings_ch1.ReceiveHandshake	= XON_XOFF_DISABLE;
        
        
        RS485EcConfig(psfgd->RS485Index, RS485_CHANNEL_1, settings_ch1);
        RS485EcConfig(psfgd->RS485Index, RS485_CHANNEL_2, settings_ch2);

        RS485EcEnable(psfgd->RS485Index, RS485_CHANNEL_1);
        RS485EcEnable(psfgd->RS485Index, RS485_CHANNEL_2);
        psfgd->FirstTime = 0;
    }else{
        /* copy input data to slave */
        RS485EcReceive(psfgd->RS485Index, RS485_CHANNEL_1, DataInSizeCh0, DataInCh0);
        RS485EcReceive(psfgd->RS485Index, RS485_CHANNEL_2, DataInSizeCh1, DataInCh1);

        /* copy output data to slave */
        RS485EcTransmit(psfgd->RS485Index, RS485_CHANNEL_1, dmin(DataOutSizeCh0,RS485_MAX_DATA_LENGTH), &BytesWritten, DataOutCh0, &FlagTxCh0);   
        RS485EcTransmit(psfgd->RS485Index, RS485_CHANNEL_2, dmin(DataOutSizeCh1,RS485_MAX_DATA_LENGTH), &BytesWritten, DataOutCh1, &FlagTxCh1);
        
        *Flag_Tx0 = FlagTxCh0;   
        *Flag_Tx1 = FlagTxCh1;

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
