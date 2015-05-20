#pragma once
#ifndef _POOL_ALLOCATOR_H
#define _POOL_ALLOCATOR_H


#define FREE_NODE_MAP_SIZE	2048
#define POOL_MAX_SIZE		65536


typedef struct __pool_allocator
{
  void *current_pool;
  void **free_node_map;
  unsigned int data_size;
  unsigned int free_node_map_index;
  unsigned int current_pool_index;
  
} pool_allocator;


pool_allocator *init_new_allocator(unsigned int);

void *get_new_node(pool_allocator*);

void free_node(pool_allocator*, void*);


#endif
