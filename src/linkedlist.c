

#include "linkedlist.h"

/**
 * @brief  Insert data into head of current mb register 
 * @return done|NULL
 */
int pushData(_ln *listNode, char *key, char *value){
  assert(listNode && key && value);
  _dn *newData = (_dn*)malloc(sizeof(_dn));
  assert(newData);
  newData->key = (char*)malloc(strlen(key));
  assert(newData->key);
  newData->value = (char*)malloc(strlen(value));
  assert(newData->value);
  strcpy(newData->key, key);
  strcpy(newData->value, value);
  newData->next = listNode->data;
  listNode->data = newData;
  return 0;
};

/**
 * @brief  Insert a new node into head of list nodes 
 * @return done|NULL
 */
int pushNode(_ln *listNode){
  assert(listNode);
  _ln *newNode = (_ln*)malloc(sizeof(_ln));
  assert(newNode);
  newNode->data = (_dn*)malloc(sizeof(_dn));
  assert(newNode->data);
//  newNode->data->key = (char*)malloc(strlen(key));
//  assert(newNode->data->key);
//  newNode->data->value = (char*)malloc(strlen(value));
//  assert(newNode->data->value);
//  strcpy(newNode->data->key, key);
//  strcpy(newNode->data->value, value);
  newNode->next = listNode;
  listNode = newNode;
  return 0;
};

/**
 * @brief  Find and return some modbus register key and value tuple
 * @param  mbr Modbus device register with all parameters tuples
 * @param  key Data tuple ID to find
 * @return _mbrTupleNode|NULL
 */
char *peekValue(_ln *listNode, char *key){
  assert(listNode->data && key);
  _dn *data = listNode->data;
  while( data ){
    if( strcmp(data->key, key) == 0 ) {       
      return data->value; 
    }
    data = data->next;
  }
  return ((char*)"0");
};

void listNode(_ln *listNode){
  assert(listNode);
  _ln *ln = listNode; 
  while ( ln ){
    _dn *data = ln->data;
    while ( data ){
      printf("%s: %s\n", data->key, data->value);
      data = data->next;
    }
    ln = ln->next;  
  }
};

void deleteData(_ln *listNode, const char *key){
  _dn *dn = listNode->data;
  _dn *prev = dn;
  assert(dn->key && dn->value);
  while( dn ) { 
    if( strcmp( dn->key, key ) == 0 ){
      prev->next = dn->next;
      free(dn->key);
      free(dn->value);
      free(dn);
      return; /* Comment to remove also duplicated data nodes */
    }
    prev = dn;
    dn = dn->next;
  }
};

void deleteNode(_ln *listNode, const char *key){
  _ln *ln = listNode;
  assert(ln && key);
  _ln *prev = ln;
  while( ln ){
    prev = ln;
    _dn *dn = ln->data;
    while(dn){
      if( strcmp(dn->key, key) == 0 ){
        prev->next = ln->next;
        while( ln->data ){
          deleteData(ln, ln->data->key);
          ln->data = ln->data->next;
        }
        free(ln->data);
        free(ln);
        return;
      }
    }
    ln = ln->next;
  }
};

