#ifdef REQUIRES_DOUBLYLINKEDLIST
  #include "./doublyLinkedList.h"
  #include <stdlib.h>
  
  
  void insertDlNodeAfter(struct dlList_t* list, struct dlNode_t* node, struct dlNode_t* newNode)
  {
    newNode->prev = node;
    newNode->next = node->next;
    
    if (node->next == NULL)
      list->lastNode = newNode;
    else
      node->next->prev = newNode;
    
    node->next = newNode;
  }
  
  void insertDlNodeBefore(struct dlList_t* list, struct dlNode_t* node, struct dlNode_t* newNode)
  {
    newNode->prev = node->prev;
    newNode->next = node;
    
    
    if (node->prev == NULL)
      list->firstNode = newNode;
    else
      node->prev->next = newNode;
    
    node->prev = newNode;
  }
  
  void insertDlNodeBeginning(struct dlList_t* list, struct dlNode_t* newNode)
  {
    if (list->firstNode == NULL)
    {
      list->firstNode = newNode;
      list->lastNode = newNode;
      
      newNode->prev = NULL;
      newNode->next = NULL;
    }
    else
      insertDlNodeBefore(list,list->firstNode,newNode);
    
  }
  
  void insertDlNodeLast(struct dlList_t* list, struct dlNode_t* newNode)
  {
    if (list->lastNode == NULL)
      insertDlNodeBeginning(list,newNode);
    else
      insertDlNodeAfter(list,list->lastNode,newNode);
  }
  
  void removeDlNode(struct dlList_t* list, struct dlNode_t* node)
  {
    if (node->prev == NULL)
      list->firstNode = node->next;
    else
      node->prev->next = node->next;
    
    if (node->next == NULL)
      list->lastNode = node->prev;
    else
      node->next->prev = node->prev;  
  }
#endif
