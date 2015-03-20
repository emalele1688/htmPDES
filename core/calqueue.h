#pragma once
#ifndef __CALQUEUE_H
#define __CALQUEUE_H


//#define CALQSPACE 49153		// Calendar array size needed for maximum resize
#define CALQSPACE 65536		// Calendar array size needed for maximum resize
#define MAXNBUCKETS 32768	// Maximum number of buckets in calendar queue


typedef struct __calqueue_node {
	double			timestamp; // Timestamp associated to the event
	void 			*payload; // A pointer to the actual content of the node
	struct __calqueue_node 	*next;		// Pointers to other nodes
		
} calqueue_node;

typedef struct __calqueue_node * calendar_queue;



extern void calqueue_init(unsigned int);

extern void *calqueue_get(unsigned int thread_id);

extern void calqueue_put(double, void *);

extern int check_safety(double, unsigned int);

extern void commit_time(unsigned int);


#endif /* __CALQUEUE_H */
