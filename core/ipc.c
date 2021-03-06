#include <stdlib.h>
#include <string.h>

#include "message_state.h"
#include "calqueue.h"

#include "pool_allocator.h"
#include "ipc.h"


#define TEMP_POOL_SIZE	32


/* Memory pool for messages send by the local thread */
struct __lts_event_pool
{
  msg_t local_pool[TEMP_POOL_SIZE];
    
  simtime_t min_time;
  
  // Counter for the non-deliver messages
  unsigned int non_deliver_size;

} __attribute__ ((aligned(64)));

typedef struct __lts_event_pool lts_event_pool;


static __thread lts_event_pool lts_pool __attribute__ ((aligned(64)));

static pool_allocator *_pool;

volatile int ipc_lock = 0;


void ipc_init(void)
{
  _pool = init_new_allocator(sizeof(msg_t));
  
  calqueue_init();
}

msg_t *next_event(void)
{
  msg_t *_msg;
  
  while(__sync_lock_test_and_set(&ipc_lock, 1))
    while(ipc_lock);

  _msg = calqueue_get();
  if(_msg == NULL)
  {
    __sync_lock_release(&ipc_lock);
    return NULL;
  }
  
  execution_time(_msg->timestamp);
    
  __sync_lock_release(&ipc_lock);
    
  return _msg;
}

double deliver_events(void)
{
  msg_t *_new;
  simtime_t min;
  unsigned int i = 0;
  
  while(i < lts_pool.non_deliver_size)
  {
    _new = get_new_node(_pool);
    memcpy(_new, &lts_pool.local_pool[i], sizeof(msg_t));
    calqueue_put(_new->timestamp, _new);
    i++;
  }
  
  min = lts_pool.min_time;
  lts_pool.min_time = INFTY;
  
  lts_pool.non_deliver_size = 0;
  
  return min;
}

void ScheduleNewEvent(unsigned int receiver, simtime_t timestamp, unsigned int event_type, void *event_content, unsigned int event_size)
{
  msg_t *new_event;
  
  if(lts_pool.non_deliver_size == THR_POOL_SIZE)
  {
    rootsim_error(true, "event pool overflow\n");
    return;
  }
  
  if(event_size > MAX_DATA_SIZE)
  {
    rootsim_error(true, "Error: requested a message of size %d, max allowed is %d\n", event_size, MAX_DATA_SIZE);
    return;
  }
   
  new_event = &lts_pool.local_pool[lts_pool.non_deliver_size++];
  
  new_event->timestamp = timestamp;
 // new_event->msg.sender_id = current_lp;
  new_event->receiver_id = receiver;
  new_event->type = event_type;
  new_event->data_size = event_size;
  
  memcpy(new_event->data, event_content, event_size);
  
  if(timestamp < lts_pool.min_time)
    lts_pool.min_time = timestamp;
}

void free_event(msg_t *msg)
{
  free_node(_pool, msg);
}









