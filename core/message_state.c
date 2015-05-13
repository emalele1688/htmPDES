#include <stdlib.h>
#include <limits.h>

#include "ipc.h"

#include "message_state.h"


struct __current_time_vector
{
  simtime_t time;
  
} __attribute__ ((aligned (64)));

struct __outgoing_time_vector
{
  simtime_t time;
  
} __attribute__ ((aligned (64)));


struct __current_time_vector *current_time_vector __attribute__ ((aligned (64)));

struct __outgoing_time_vector *outgoing_time_vector  __attribute__ ((aligned (64)));


extern __thread unsigned int events;


void message_state_init(void)
{
  int i, ret;
  
  if( (ret = posix_memalign((void**)&current_time_vector, 64, sizeof(struct __current_time_vector) * n_cores)) != 0 || 
    (ret = posix_memalign((void**)&outgoing_time_vector, 64, sizeof(struct __current_time_vector) * n_cores)) != 0)
  {
    rootsim_error(true, strerror(ret));
    abort();
  }
  
  for(i = 0; i < n_cores; i++)
  {
    current_time_vector[i].time = INFTY;
    outgoing_time_vector[i].time = INFTY;
  }
}

void execution_time(simtime_t current_time)
{    
  current_time_vector[tid].time = current_time;
  outgoing_time_vector[tid].time = INFTY;
}

void commit_time(simtime_t min_time_output)
{
  current_time_vector[tid].time = INFTY;
  outgoing_time_vector[tid].time = min_time_output;
}

int check_safety(void)
{
  int i;
  unsigned int min_tid = n_cores + 1;
  double min = INFTY;
  int ret = 0;

  events = 0;
  
  for(i = 0; i < n_cores; i++)
  {
    if( (i != tid) && ((current_time_vector[i].time < min) || (outgoing_time_vector[i].time < min)) )
    {
      min = ( current_time_vector[i].time < outgoing_time_vector[i].time ?  current_time_vector[i].time : outgoing_time_vector[i].time  );
      min_tid = i;
      events++;
    }
  }

  if((current_time_vector[tid].time < min) || (current_time_vector[tid].time == min && tid < min_tid))
    ret = 1;
  
  return ret;
}
