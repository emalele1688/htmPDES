#ifndef __CORE_H
#define __CORE_H


#include <float.h>
#include <stdbool.h>
#include <math.h>

#include "ROOT-Sim.h"


#define MAX_LPs			2048


#define D_DIFFER_ZERO(a) (fabs(a) >= DBL_EPSILON)


extern __thread simtime_t current_lvt;
extern __thread unsigned int current_lp;
extern __thread unsigned int tid;

extern unsigned int n_cores;
extern unsigned int n_prc_tot;


void init(unsigned int _thread_num, unsigned int);

void rootsim_error(bool fatal, const char *msg, ...);

void _mkdir(const char *path);

void SetState(void *ptr);

//Esegue il loop del singolo thread
void thread_loop(unsigned int thread_id);

extern void ProcessEvent(unsigned int me, simtime_t now, unsigned int event, void *content, unsigned int size, void *state);

extern int OnGVT(unsigned int me, void *snapshot);


#endif
