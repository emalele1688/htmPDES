#pragma once
#ifndef __IPC_H
#define __IPC_H


#include "core.h"


#define POOL_START_SIZE		1024
#define MAX_DATA_SIZE		32
#define THR_POOL_SIZE		32



struct __msg_t
{  
  simtime_t timestamp;			// 0  - 8
  
  unsigned int sender_id;		// 8  - 12
  unsigned int receiver_id;		// 12 - 16
  
  int type;				// 16 - 20
  
  unsigned int data_size;		// 20 - 24
  unsigned char data[MAX_DATA_SIZE];	// 24 - 64
  
}; 
 
typedef struct __msg_t msg_t;


extern int ipc_lock;


void ipc_init(void);

msg_t *next_event(void);

void free_event(msg_t *msg);

double deliver_events(void);

void ScheduleNewEvent(unsigned int receiver, simtime_t timestamp, unsigned int event_type, void *event_content, unsigned int event_size);


#endif
