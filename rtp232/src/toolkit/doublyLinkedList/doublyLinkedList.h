#ifdef REQUIRES_DOUBLYLINKEDLIST

  #ifndef DOUBLYLINKEDLIST_H_
  #define DOUBLYLINKEDLIST_H_

  // use this as template and cast!...
  struct dlNode_t
  {
    struct dlNode_t* prev;
    struct dlNode_t* next;  
    //void* data;
  };

  struct dlList_t
  {
    struct dlNode_t* firstNode;
    struct dlNode_t* lastNode;
  };


  void insertDlNodeAfter(struct dlList_t* list, struct dlNode_t* node, struct dlNode_t* newNode);
  void insertDlNodeBefore(struct dlList_t* list, struct dlNode_t* node, struct dlNode_t* newNode);
  void insertDlNodeBeginning(struct dlList_t* list, struct dlNode_t* newNode);
  void insertDlNodeLast(struct dlList_t* list, struct dlNode_t* newNode);
  void removeDlNode(struct dlList_t* list, struct dlNode_t* node);

  #endif /*DOUBLYLINKEDLIST_H_*/
#endif
