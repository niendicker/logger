#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct __dn{  /* Generic data node for single linked list */
  void *key;   /* Used to update and retrieve data */
  void *value; /* Data value */
  struct __dn  *next;
} _dn;

typedef struct __ln { /* Generic list node for single linked list */
  _dn *data;
  struct __ln *next;
} _ln;

/**
 * @brief  Insert data into head of current mb register 
 * @return done|NULL
 */
int pushData(_ln *listNode, char *key, char *value){
  assert(listNode && key && value);
  _dn *dataNode = (_dn*)malloc(sizeof(_dn));
  assert(dataNode);
  dataNode->key = (char*)malloc(strlen(key));
  assert(dataNode->key);
  dataNode->value = (char*)malloc(strlen(value));
  assert(dataNode->value);
  strcpy((char*)dataNode->key, key);
  strcpy((char*)dataNode->value, value);
  dataNode->next = listNode->data;
  listNode->data = dataNode;
  return 0;
};

/**
 * @brief  Insert a new node into head of list nodes 
 * @return done|NULL
 */
int pushNode(_ln *listNode, char *key, char *value){
  assert(listNode && key && value);
  _ln *newNode = (_ln*)malloc(sizeof(_ln));
  assert(newNode);
  newNode->data->key = (char*)malloc(strlen(key));
  assert(newNode->data->key);
  newNode->data->value = (char*)malloc(strlen(value));
  assert(newNode->data->value);
  strcpy((char*)newNode->data->key, key);
  strcpy((char*)newNode->data->value, value);
  newNode->next = listNode;
  listNode = newNode;
  return 0;
};