#ifdef REQUIRES_QUEUE
  #include <stdint.h>
  #include <stdlib.h>
  #include "./queue.h"
  
  // TODO add maximum element counter....
  
  // a queue has a limited size, therefore allways call queueIsFull before 
  // adding a node. Adding a node to a full is, will be rejected.
  // Inorder to prevent memory holes always make sure that the node has been added.
  uint8_t pushQueueEntry(struct queue_t* queue, struct queueNode_t* node)
  {
    // reject adding elements to a full queue...
    if (queue->elements == 0)      
      return 1;
    
    // when pushing a new Entry ontop of the queue...
    // ... therefore the node can't have a successor.
    node->prev = NULL;
                 
    if (queue->first == NULL)
      queue->last = node;
    else        
      queue->first->prev = node;
          
    queue->first = node;
    
    queue->elements--;
    
    return 0;
  }
  
  uint8_t queueIsEmpty(struct queue_t* queue)
  {
    if (queue->last == NULL)
      return 0;
    
    return 1;    
  }

  uint8_t queueIsFull(struct queue_t* queue)
  {
    if (queue->elements > 0)
      return 0;
    
    return 1;    
  }
  
  struct queueNode_t* popQueueEntry(struct queue_t* queue)
  {
    if (queue->last == NULL)
      return NULL;
    
    struct queueNode_t* node = queue->last;
    
    queue->last = queue->last->prev;
    
    if (queue->last == NULL)
      queue->first = NULL;
    
    queue->elements++;
    
    return node;
  }
#endif
