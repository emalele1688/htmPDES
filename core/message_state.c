#include <stdlib.h>
#include <limits.h>

#include "ipc.h"

#include "message_state.h"



simtime_t *current_time_vector;

simtime_t *outgoing_time_vector;

extern int queue_lock;


void message_state_init(void)
{
  int i;
  
  current_time_vector = malloc(sizeof(simtime_t) * n_cores);
  outgoing_time_vector = malloc(sizeof(simtime_t) * n_cores);
  
  for(i = 0; i < n_cores; i++)
  {
    current_time_vector[i] = INFTY;
    outgoing_time_vector[i] = INFTY;
  }
}

void execution_time(simtime_t current_time)
{    
  current_time_vector[tid] = current_time;
  outgoing_time_vector[tid] = INFTY;
}

void commit_time(simtime_t min_time_output)
{
  current_time_vector[tid] = INFTY;
  outgoing_time_vector[tid] = min_time_output;
}

int check_safety(unsigned int *events)
{
  int i;
  unsigned int min_tid = n_cores + 1;
  double min = INFTY;
  int ret = 0;

  *events = 0;
  
  for(i = 0; i < n_cores; i++)
  {
    if( (i != tid) && ((current_time_vector[i] < min) || (outgoing_time_vector[i] < min)) )
    {
      min = ( current_time_vector[i] < outgoing_time_vector[i] ?  current_time_vector[i] : outgoing_time_vector[i]  );
      min_tid = i;
      *events++;
    }
  }

  if((current_time_vector[tid] < min) || (current_time_vector[tid] == min && tid < min_tid))
    ret = 1;
  
  return ret;
}
