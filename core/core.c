#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <string.h>
#include <immintrin.h>
#include <stdarg.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ROOT-Sim.h"

#include "ipc.h"
#include "message_state.h"

#include "dymelor.h"
#include "numerical.h"
#include "timer.h"

#include "core.h"


// Main thread id
#define _MAIN_THREAD		0
// Abort code for cross check condition failure
#define _ROLLBACK_CODE		127

#define MAX_PATHLEN	512

#define HILL_EPSILON_GREEDY	0.05

#define HILL_CLIMB_EVALUATE	500

#define DELTA 500  // tick count

#define HIGHEST_COUNT	5


// LPs-state structure pointer (each LP allocates the state structure locally)
static void **states;

// Current local virtual time
__thread simtime_t current_lvt = 0;
// Lp executed by the current thread
__thread unsigned int current_lp = 0;
// Current thread id
__thread unsigned int tid = 0;

__thread unsigned int events __attribute__ ((aligned (64))) = 0;

static __thread double abort_percent = 1.0;

static __thread unsigned long int evt_count = 0;

static __thread unsigned long int abort_count_conflict = 0, abort_count_safety = 0;

static __thread int delta_count = 0;

// Total number of cores required for simulation 
unsigned int n_cores;
// Total number of logical processes running in the simulation 
unsigned int n_prc_tot;

// Controll flag for main loop
static bool stop = false;
// LP execution flag
static bool *can_stop;
// System error flag
static bool sim_error = false;


static void process_init_event(void);

static void throttling(void);

static void hill_climbing(void);

static bool check_termination(void);

static void flush(void);


void rootsim_error(bool fatal, const char *msg, ...) 
{
  char buf[1024];
  va_list args;

  va_start(args, msg);
  vsnprintf(buf, 1024, msg, args);
  va_end(args);

  fprintf(stderr, (fatal ? "[FATAL ERROR] " : "[WARNING] "));

  fprintf(stderr, "%s", buf);
  fflush(stderr);

  if(fatal)
    sim_error = true;
}

void _mkdir(const char *path)
{
  char opath[MAX_PATHLEN];
  char *p;
  size_t len;

  strncpy(opath, path, sizeof(opath));
  len = strlen(opath);
  if(opath[len - 1] == '/')
  opath[len - 1] = '\0';

  // opath plus 1 is a hack to allow also absolute path
  for(p = opath + 1; *p; p++) 
  {
    if(*p == '/') 
    {
      *p = '\0';
      if(access(opath, F_OK))
	if(mkdir(opath, S_IRWXU))
	  if(errno != EEXIST)
	    rootsim_error(true, "Could not create output directory", opath);
      *p = '/';
    }
  }

  // Path does not terminate with a slash
  if(access(opath, F_OK))
    if(mkdir(opath, S_IRWXU))
      if(errno != EEXIST)
	rootsim_error(true, "Could not create output directory", opath);
}

void SetState(void *ptr)
{
  states[current_lp] = ptr;
}

static void process_init_event(void) 
{
  unsigned int i;

  for(i = 0; i < n_prc_tot; i++) 
  {
    current_lp = i;
    current_lvt = 0;
    ProcessEvent(current_lp, current_lvt, INIT, NULL, 0, states[current_lp]);
    deliver_events(); 
  }
}

void init(unsigned int thread_num, unsigned int lps_num)
{
  printf("Starting an execution with %u threads, %u LPs\n", thread_num, lps_num);
  n_cores = thread_num;
  n_prc_tot = lps_num;
  
  states = malloc(sizeof(void *) * n_prc_tot);
  can_stop = malloc(sizeof(bool) * n_prc_tot);
 
#ifndef NO_DYMELOR
  dymelor_init();
#endif
  ipc_init();
  message_state_init();
  numerical_init();
  
  process_init_event();
}

static void throttling(void) 
{
  long long tick_count;
  register int i;

  if(delta_count == 0)
    return;

  tick_count = CLOCK_READ();
  while(true) 
    if(CLOCK_READ() > tick_count + (events * DELTA * delta_count))
      break;
}

static void hill_climbing(void) 
{
  if(((double)abort_count_safety / (double)evt_count) < abort_percent && delta_count < HIGHEST_COUNT)
    delta_count++;

  abort_percent = (double)abort_count_safety / (double)evt_count;
}

bool check_termination(void)
{
  int i;
  bool ret = true;
  
  for(i = 0; i < n_prc_tot; i++)
    ret &= can_stop[i];

  return ret;
}

void thread_loop(unsigned int thread_id)
{
  msg_t *current_msg;
  int status;
   
  tid = thread_id;
  
  while(!stop && !sim_error)
  {
    if( (current_msg = next_event()) == NULL)
      continue; 

    current_lp = current_msg->receiver_id;
    current_lvt  = current_msg->timestamp;

    while(1)
    {
      if(check_safety())
	ProcessEvent(current_lp, current_lvt, current_msg->type, current_msg->data, current_msg->data_size, states[current_lp]);
      else
      {
	if( (status = _xbegin()) == _XBEGIN_STARTED)
	{
	  ProcessEvent(current_lp, current_lvt, current_msg->type, current_msg->data, current_msg->data_size, states[current_lp]);

	  #ifdef THROTTLING
          throttling();
	  #endif 
	  
	  if(check_safety())
	    _xend();
	  else
	    _xabort(_ROLLBACK_CODE);
	}
	else
	{
	  status = _XABORT_CODE(status);
	  if(status == _ROLLBACK_CODE)
	    abort_count_conflict++;
	  else
	    abort_count_safety++;
	  continue;
	}
      }
      
      break;
    }

    free_event(current_msg);

    flush();

    #ifdef THROTTLING
    if((evt_count - HILL_CLIMB_EVALUATE * (evt_count / HILL_CLIMB_EVALUATE)) == 0)
      hill_climbing();
    #endif
    
    can_stop[current_lp] = OnGVT(current_lp, states[current_lp]);
    stop = check_termination();

    if(tid == _MAIN_THREAD) 
    {
      evt_count++;
      if((evt_count - 10000 * (evt_count / 10000)) == 0)
	printf("TIME: %f\n", current_lvt);
    }
  }
  
  printf("Thread %u aborted %lu times for cross check condition and %lu for memory conflicts\n", tid, abort_count_conflict, abort_count_safety);
}

void flush(void) 
{
  double t_min;
  while(__sync_lock_test_and_set(&ipc_lock, 1))
    while(ipc_lock);

  t_min = deliver_events();

  commit_time(t_min);

  __sync_lock_release(&ipc_lock);
}



