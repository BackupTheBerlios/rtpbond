#ifdef REQUIRES_QUEUE
  #ifndef QUEUE_H_
  #define QUEUE_H_
  
    struct queue_t
    {  
      struct queueNode_t* first;
      struct queueNode_t* last;
      uint8_t elements;
    };
   
    struct queueNode_t
    {
      struct queueNode_t* prev;
      //char* data;
      //uint8_t length;
    };  
    
    //struct soFiFoHandle_t* fifoNewBuffer();
    struct queueNode_t* popQueueEntry(struct queue_t* queue);
    uint8_t queueIsEmpty(struct queue_t* queue);
    uint8_t queueIsFull(struct queue_t* queue);
    uint8_t pushQueueEntry(struct queue_t* queue, struct queueNode_t* node);
    
  #endif /*QUEUE_H_*/
#endif
