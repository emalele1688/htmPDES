#ifndef __CORE_H
#define __CORE_H


#include "ROOT-Sim.h"

#include <stdbool.h>
#include <math.h>
#include <float.h>


<<<<<<< HEAD
#define MAX_LPs			2048
=======
#include <float.h>

#include <ROOT-Sim.h>

#define MAX_LPs	2048

#define MAX_DATA_SIZE		64
#define THR_POOL_SIZE		20
>>>>>>> parent of 6173fc2... Some tuning

#define D_DIFFER_ZERO(a) (fabs(a) >= DBL_EPSILON)


extern __thread simtime_t current_lvt;
extern __thread unsigned int current_lp;
extern __thread unsigned int tid;

extern unsigned int n_cores;
extern unsigned int n_prc_tot;


void init(unsigned int _thread_num, unsigned int);

//Esegue il loop del singolo thread
void thread_loop(unsigned int thread_id);

extern void rootsim_error(bool fatal, const char *msg, ...);

extern void _mkdir(const char *path);

#endif
