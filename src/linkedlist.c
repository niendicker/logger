

#include "linkedlist.h"

/**
 * @brief  Insert data into head of current mb register 
 * @return done|NULL
 */
_ln *pushData(_ln *listNode, char *key, char *value){
  assert(key && value);
  _dn *newData = (_dn*)calloc(sizeof(_dn), _byte_size_);
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
 * @brief  Update the value of a given key
 * @return _mbrTupleNode|NULL
 */
char *updateValue(_ln *listNode, char* value){
  assert(listNode->data && value);
  uint64_t hash = djb2_hash((char*)"lastValid");
  _dn *data = listNode->data;
  while( data ){
    if( hash == data->keyHash ) {       
      data->value = srealloc_copy(data->value ,value); 
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
void deleteData(_dn *data, const char *key){
  _dn *dn = data;
  uint64_t hash = djb2_hash(key);
  while( dn ){
    if(hash == dn->keyHash){
      free(dn->key);
      free(dn->value);
      free(dn);
      return;
    }
    dn = dn->next;
  }
};

/**
 * @brief  Delete the first node that contains the specified key in data 
 */
void deleteNode(_ln *listNode, const char *key){
  assert(listNode && key);
  uint64_t hash = djb2_hash(key);
  _ln *node = listNode;
  while(node){
    _dn *dn = node->data;
    if( dn->keyHash == hash ){
      while(dn){
        deleteData(dn, dn->key);
        dn = dn->next;
      }
      node = node->next;
      return;
    }
    node = node->next;
  }
};

