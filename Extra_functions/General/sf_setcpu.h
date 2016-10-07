
#define S_FUNCTION_NAME  sf_setcpu
#define S_FUNCTION_LEVEL 2

/* header files */
#include "simstruc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sched.h>
#include <math.h>
#include "rtdb/rtdb_api.h"
#include <unistd.h>

/* macro definitions */
#define NINPORTS	0
#define NOUTPORTS	1
#define NOUTPUTS        1
#define NPARAMS         1

#define SFPAR_CPU_ID(S)	ssGetSFcnParam(S, 0)

#ifdef MATLAB_MEX_FILE
#define print	mexPrintf
#else
#define print	printf
#endif

typedef struct tag_sfun_global_data {
    int_T first_time;
    int_T cpu_id;
    cpu_set_t mask;
    int_T Ncores;
} sfun_global_data, *psfun_global_data;

