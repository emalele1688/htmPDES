#pragma once
#ifndef __MESSAGESTATE_H
#define __MESSAGESTATE_H


#include "core.h"



void message_state_init(void);

// It sets the actual execution time in the current thread
void execution_time(simtime_t time);

// Commit the actual execution time in the current thread
void commit_time(simtime_t time);

// It returns 1 if there is not any other timestamp less than the current timestamp executed in this thread
int check_safety(void);


#endif
