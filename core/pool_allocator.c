#include <stdlib.h>
#include <string.h>

#include "pool_allocator.h"


static void alloc_new_pool(pool_allocator *_pool);


void alloc_new_pool(pool_allocator* _pool)
{
  _pool->current_pool = malloc(_pool->data_size * POOL_MAX_SIZE);
  _pool->current_pool_index = 0;
}

pool_allocator *init_new_allocator(unsigned int type_size)
{
  void *_ptr;
  pool_allocator *header;
  
  _ptr = malloc(sizeof(pool_allocator) + (sizeof(void*) * FREE_NODE_MAP_SIZE));
  
  header = (pool_allocator*)_ptr;
  header->free_node_map = _ptr + sizeof(pool_allocator);
  header->data_size = type_size;
  header->free_node_map_index = 0;
  
  alloc_new_pool(header);
  
  return header;
}

/* It is not thread-safe. get_new_node it's called into an atomic operation */
void *get_new_node(pool_allocator *_pool)
{
  void *_ret;
  volatile unsigned int aux, curr;
  
  if(_pool->current_pool_index == POOL_MAX_SIZE)
    alloc_new_pool(_pool);

free_node_alloc_retry:
  if(_pool->free_node_map_index > 0)
  {
    aux = _pool->free_node_map_index;
    curr = __sync_fetch_and_sub(&aux, 1);
    if(!__sync_bool_compare_and_swap(&_pool->free_node_map_index, curr, aux))
      goto free_node_alloc_retry;
    
    _ret = (void*)(_pool->free_node_map[aux]);
  }
  else
  {
    _ret = (void*)(((char*)_pool->current_pool) + (_pool->current_pool_index * _pool->data_size)); 
    _pool->current_pool_index++;
  }

  return _ret;
}

/* thread-safe free */
void free_node(pool_allocator *_pool, void *_ptr)
{
  volatile unsigned int curr, aux;

free_node_dealloc_retry:
  if(_pool->free_node_map_index < FREE_NODE_MAP_SIZE)
  {
    aux = _pool->free_node_map_index;
    curr = __sync_fetch_and_add(&aux, 1);
    if(!__sync_bool_compare_and_swap(&_pool->free_node_map_index, curr, aux))
      goto free_node_dealloc_retry;
    
    memcpy(&_pool->free_node_map[curr], (void*)&_ptr, sizeof(void*));
  }
}




