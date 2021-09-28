#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
  Linked List Map Data structure 
  listNode->next --------> listNode->next ------> listNode->next = NULL 
  +-->data                 +-->data
      +-->Key                  +-->Key
      +-->Value                +-->Value         
      +-->data->next----+      +-->data->next----+
                data <--+                data <--+      
                +-->Key                  +--> Key    
                +-->Value                +--> Value 
                +-->data->next----+      +--> data->next----+ 
                          NULL <--+                 NULL <--+
*/

typedef struct __dn{  /* Generic data node for single linked list */
  char *key;   /* Used to update and retrieve data */
  char *value; /* Data value */
  struct __dn  *next;
} _dn;

typedef struct __ln { /* Generic list node for single linked list */
  _dn *data;
  struct __ln *next;
} _ln;

/**
 * @brief  Insert a new node into head of list nodes 
 * @return done|NULL
 */
int pushNode(_ln *listNode);

/**
 * @brief  Insert data into head of current mb register 
 * @return done|NULL
 */
int pushData(_ln *listNode, char *key, char *value);

/**
 * @brief  Find and return some modbus register key and value tuple
 * @param  mbr Modbus device register with all parameters tuples
 * @param  key Data tuple ID to find
 * @return _mbrTupleNode|NULL
 */
char *peekValue(_ln *listNode, char *key);

/**
 * @brief  Print to stdout the data of a list node
 */
void listNode(_ln *listNode);

void deleteData(_ln *listNode, const char *key);


void deleteNode(_ln *listNode, const char *key);


#endif /* linkedlist.h */