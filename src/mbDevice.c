/**
 * @file   device_config_parser.c
 * @author Marcelo Niendicker Grando
 * @date   2021/09/13
 * @brief  Load device configuration and registers map
 *
 * @see Modbus specification
 * @see https://modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
 * @see https://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf
 */

#include "mbDevice.h"

int deviceInit(device *dev){
  memset(dev, 0, sizeof(*dev));
  return done;
};

/**
 * @brief Convert/Store all "non string" values on current context for fastest access
 */
int deviceSetCtx(device *mbDevice){
  assert( mbDevice && mbDevice->config );
  _config *config = mbDevice->config;
  mbDevice->link.protocol              = (uint16_t) strtol( confValue( config, protocol   ), NULL, 10 );
  mbDevice->link.modbusTcp.port        = (uint16_t) strtol( confValue( config, port       ), NULL, 10 );
  mbDevice->link.modbusTcp.msTimeout   = (uint16_t) strtol( confValue( config, msTimeout  ), NULL, 10 );
  mbDevice->link.modbusRtu.unitAddress = (uint8_t ) strtol( confValue( config, unitAddress), NULL, 10 );
  mbDevice->link.modbusRtu.baudRate    = (uint16_t) strtol( confValue( config, baudRate   ), NULL, 10 );
  mbDevice->link.modbusRtu.dataBits    = (uint8_t ) strtol( confValue( config, dataBits   ), NULL, 10 );
  mbDevice->link.modbusRtu.stopBits    = (uint8_t ) strtol( confValue( config, stopBits   ), NULL, 10 );
  mbDevice->link.modbusRtu.parity      = (uint8_t ) strtol( confValue( config, parity     ), NULL, 10 );
  mbDevice->link.modbusRtu.handshake   = (uint8_t ) strtol( confValue( config, handshake  ), NULL, 10 );
  return done;
}

/**
 * @brief Load all device parameters
 */
device *deviceConfigure(device* mbDevice, const char* filePath){
  assert(mbDevice && filePath);
  FILE* deviceConf = fopen(filePath, "r");
  assert(deviceConf);
  char keyValue[200];
  _config *newConfig = (_config*)malloc(sizeof(_config));
  assert(newConfig);
  for( int line = 1; fgets(keyValue, sizeof(keyValue), deviceConf) != NULL; line++){ /* search for key = value lines on file */
    if( IGNORE( keyValue[0] ) ) continue; /* Comment or bad format */
    char* token = strtok(keyValue, "= ");
    assert(token);
    char* value = strtok( NULL, "= " );
    assert(value);
    value[ strcspn(value, "\n") ] = '\0'; /* Remove new line from value  */  
    _dn *data = (_dn*)malloc(sizeof(_dn));
    data->_key = (char*)malloc(strlen(token));
    assert(data->_key);
    data->_value = (char*)malloc(strlen(value));
    assert(data->_value);
    strcpy(data->_key, token);
    strcpy(data->_value, value);
    data->_next = newConfig->_data;
    newConfig->_data = data;
  } /* File scan */
  newConfig->_next = mbDevice->config;
  mbDevice->config = newConfig;
  deviceSetCtx(mbDevice);
  fclose(deviceConf);
  return(mbDevice);
}

/**
 * @brief  Print device configuration parameters
 */
int showDeviceConf(device *dev){
  assert(dev && dev->config); 
  printf("Info: Device configuration\n");
  _config *_c = dev->config; 
  while (_c->_data->_key){
    char * key = (char*)_c->_data->_key;
    char * value = (char*)_c->_data->_value;
    printf("%s: %s\n", key, value);
    _c->_data = _c->_data->_next;
  }  
  return done;
};

/**
 * @brief  Find and return some device configuration key and value tuple
 */
char *confPeekValue(_config *config, char *key){
  assert(config && key);
  _config *_c = config; 
  while (_c->_data->_key) {
    if( strcmp((char*)_c->_data->_key, key) == 0 ) { 
      char *v = (char*)malloc(strlen(_c->_data->_value));
      assert(v);
      strcpy(v, _c->_data->_value);
      return v; 
    }
    _c->_data = _c->_data->_next;
  }
  return NULL;
};

/**
 * @brief  Free all dynamic memory allocated
**/
int freeDeviceConf(device *dev){
//  _confNode *confNodeTmp = NULL, *confNode = dev->currConfig;
//  while (confNode) {
//    _confDataNode *tNodeTmp=NULL, *tNode=confNode->_head;    
//    while (tNode) {
//      tNodeTmp = tNode;
//      tNode = tNode->next;
//      free(tNodeTmp->key);
//      free(tNodeTmp->value);
//    }  
//    confNode = confNode->next;
//    free(confNodeTmp);
//  };
//  return done;
  return failure;
};

/**
 * @brief Load all registers map information of device using linked lists
 *
 * @param dev Device with registers map file path loaded on loadDeviceConf()
 * @return device|NULL
 */
device *deviceMap(device *dev){
  assert(dev);
  FILE *deviceMap = fopen(confValue(dev->config, mapFile), "r");
  if( !deviceMap ) {
    printf("Error: Can't open device registers map file");
    return NULL;
  }  
  for (char _kv[100]; fgets(_kv, sizeof(_kv), deviceMap) != NULL;) { /*  Scan the entire file */
    if( ( IGNORE( _kv[0] ) ) || ( ! TOKEN_KEY_MATCH( _kv, startTag) ) ) {
         continue; /* Comments */
    }/* Found a register start tag */
    _mbr *newMbr = (_mbr*)malloc(sizeof(_mbr)); /* Create a new register */
    assert(newMbr); 
    for( int i = 0; i < _lastTuple_; i++) {  
      if( ( fgets( _kv, sizeof(_kv), deviceMap ) == NULL ) ||
          ( IGNORE( _kv[0] ) ) ) { /*INVALID characters aren't allowed. Abort scanning... */
        printf("Error: Invalid modbus register data block: %s\n", _kv);
        return NULL;
      }
      char* token = strtok(_kv, "= ");
      assert(token);
      char *value = strtok(NULL, "= ");
      assert(value);
      value[strcspn(value, "\n")] = '\0'; /* Remove new line from value */
      mbrMapPushData(newMbr , token, value);
    } 
    newMbr->_next = dev->mbr;
    dev->mbr = newMbr;
  } /* Scan entire file for registers data blocks */
  fclose(deviceMap);
  return(dev);
} /* loadDeviceMap */

/**
 * @brief  Print device registers map information
 * @param  dev Modbus device 
 * @return done|failure
 */
int showDeviceMap(device *dev) {
  assert(dev);
  _mbr *mbr = dev->mbr;
  while(mbr){
    printf("\nInfo: Modbus Register\n");
    while (mbr->_data->_key){
      char * key = (char*)mbr->_data->_key;
      char * value = (char*)mbr->_data->_value;
      printf("%s:%s|", key, value);
      mbr->_data = mbr->_data->_next;
    }
    mbr = mbr->_next;
  }
  puts("\n");
  return done;
};

typedef struct __ln {
  _dn *data;
  struct __ln *next;
} _ln;

/**
 * @brief  Insert data into head of current mb register 
 * @return done|NULL
 */
int pushData(_ln *listNode, void *key, void *value){
  assert(listNode && key && value);
  _dn *newData = (_dn*)malloc(sizeof(_dn));
  assert(newData);
  newData->_key = (char*)malloc(strlen(key));
  assert(newData->_key);
  newData->_value = (char*)malloc(strlen(value));
  assert(newData->_value);
  strcpy(newData->_key, key);
  strcpy(newData->_value, value);
  newData->_next = listNode->data;
  listNode->data = newData;
  return done;
};

/**
 * @brief  Insert a new node into head of list nodes 
 * @return done|NULL
 */
int pushNode(_ln *listNode, void *key, void *value){
  assert(listNode && key && value);
  _ln *newNode = (_ln*)malloc(sizeof(_ln));
  assert(newNode);
  newNode->data = (_dn*)malloc(sizeof(_dn));
  assert(newNode->data);
  newNode->data->_key = (char*)malloc(strlen(key));
  assert(newNode->data->_key);
  newNode->data->_value = (char*)malloc(strlen(value));
  assert(newNode->data->_value);
  strcpy(newNode->data->_key, key);
  strcpy(newNode->data->_value, value);
  newNode->next = listNode;
  listNode = newNode;
  return done;
};

/**
 * @brief  Insert data into head of current mb register 
 * @return done|NULL
 */
int mbrMapPushData(_mbr *mbr, char *key, char *value){
  assert(mbr && key && value);
  _dn *newData = (_dn*)malloc(sizeof(_dn));
  assert(newData);
  newData->_key = (char*)malloc(strlen(key));
  assert(newData->_key);
  newData->_value = (char*)malloc(strlen(value));
  assert(newData->_value);
  strcpy(newData->_key, key);
  strcpy(newData->_value, value);
  newData->_next = mbr->_data;
  mbr->_data = newData;
  return done;
};

/**
 * @brief  Find and return some modbus register key and value tuple
 * @param  mbr Modbus device register with all parameters tuples
 * @param  key Data tuple ID to find
 * @return _mbrTupleNode|NULL
 */
char *mbrPeekValue(_mbr *mbr, char *key){
  assert(mbr && key);
  _dn *data = mbr->_data;
  while (data->_key){
    if( strcmp(data->_key, key) == 0 ) { 
      char *v = (char*)malloc(strlen(data->_value));
      assert(v);
      strcpy(v, mbr->_data->_value);
      return v; 
    }
    data = data->_next;
  }
  return NULL;
};

/**
 * @brief  Free all dynamic memory allocated
 * @param  dev Structure with memory to be deallocated
 * @return done|failure
 */
int freeDeviceMap(device *dev){
//  _mbrNode *mbrNodeTmp = NULL, *mbrNode = dev->headMap;
//  while (mbrNode) {
//    _mbrMap *tNodeTmp=NULL, *tNode=mbrNode->_head;    
//    while (tNode) {
//      tNodeTmp = tNode;
//      tNode = tNode->next;
//      free(tNodeTmp->key);
//      free(tNodeTmp->value);
//    }  
//   dev->currMapMbr =dev->currMapMbr->next;
//    free(mbrNodeTmp);
//  };
  return done;
}