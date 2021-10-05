

#include "linkedlist.h"

/**
 * @brief  Insert data into head of current mb register 
 * @return done|NULL
 */
_ln *pushData(_ln *listNode, char *key, char *value){
  assert(key && value);
  _dn *newData = (_dn*)malloc(sizeof(_dn));
  assert(newData);
  newData->keyHash = djb2_hash(key);
  newData->key = salloc_init(key);
  newData->value = salloc_init(value);
  newData->next = listNode->data;
  listNode->data = newData;
  assert(newData);
  return listNode;
};

/**
 * @brief  Insert a new node into head of list nodes 
 * @return done|NULL
 */
_ln *pushNode(_ln *listNode){
  _ln *newNode = (_ln*)calloc(sizeof(_ln), _byte_size_);
  assert(newNode);
  newNode->next = listNode;
  listNode = newNode;
  assert(listNode);
  return listNode;
};

/**
 * @brief  Find and return some modbus register key and value tuple
 * @return _mbrTupleNode|NULL
 */
char *peekValue(_ln *listNode, char *key){
  assert(listNode->data && key);
  uint64_t hash = djb2_hash(key);
  _dn *data = listNode->data;
  while( data ){
    if( hash == data->keyHash ) {       
      return data->value; 
    }
    data = data->next;
  }
  return NULL;
};

/**
 * @brief  Print to stdout the data of a given list node
 */
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

/**
 * @brief  Delete the data specified by key on any list node
 */
void deleteData(_ln *listNode, const char *key){
  _dn *dn = listNode->data, *prev;
  uint64_t hash = djb2_hash(key);
  if(dn != NULL && ( hash == dn->keyHash )){
    listNode->data = dn->next; 
    free(dn);
    return;
  }
  while( dn != NULL &&  hash != dn->keyHash) { 
    prev = dn;
    dn = dn->next;
  }
  if(dn == NULL) return;
  prev->next = dn->next;
  free(dn->key);
  free(dn->value);
  free(dn);
};

/**
 * @brief  Delete the first node that contains the specified key in data 
 */
void deleteNode(_ln *listNode, const char *key){
  assert(listNode && key);
  uint64_t hash = djb2_hash(key);
  _ln *ln = listNode, *prev;
  while( ln ){
    prev = ln;
    _dn *dn = ln->data;
    while(dn){
      if( dn->keyHash == hash ){
        deleteData(ln, dn->key);
        dn = dn->next;
        prev->next = ln->next;
        free(ln);
        return;
      }
    }
    ln = ln->next;
  }
};

