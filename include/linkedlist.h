#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "resources.h"

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
  uint64_t keyHash;
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
_ln *pushNode(_ln *listNode);

/**
 * @brief  Insert data into head of current mb register 
 * @return done|NULL
 */
_ln *pushData(_ln *listNode, char *key, char *value);

/**
 * @brief  Find and return some modbus register key and value tuple
 * @param  mbr Modbus device register with all parameters tuples
 * @param  key Data tuple ID to find
 * @return _mbrTupleNode|NULL
 */
char *peekValue(_ln *listNode, char *key);

/**
 * @brief  Update the value of a given key
 * @return _mbrTupleNode|NULL
 */
char *updateValue(_ln *listNode, char* value);

/**
 * @brief  Print to stdout the data of a given list node
 */
void listNode(_ln *listNode);

/**
 * @brief  Delete the data specified by key on any list node
 */
void deleteData(_dn *data, const char *key);

/**
 * @brief  Delete the first node that contains the specified key in data 
 */
void deleteNode(_ln *listNode, const char *key);

#endif /* linkedlist.h */