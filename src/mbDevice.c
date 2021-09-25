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

int deviceSetCtx(device *mbDevice){
  assert( mbDevice && mbDevice->currConfig );
  _confNode *confNode = mbDevice->currConfig;
  strcpy ( mbDevice->mapFile, confValue(confNode, mapFile) );
  strcpy ( mbDevice->tag, confValue(confNode, tag) );
  strcpy ( mbDevice->link.protocol, confValue(confNode, protocol) );
  strcpy ( mbDevice->link.modbusTcp.ipAddress, confValue(confNode, ipAddress) );
  strcpy ( mbDevice->link.modbusTcp.hostname, confValue(confNode, hostname) );
  mbDevice->link.modbusTcp.port        = (uint16_t)strtol( confValue( confNode, port      ) , NULL, 10);
  mbDevice->link.modbusTcp.msTimeout   = (uint16_t)strtol( confValue( confNode, msTimeout ) , NULL, 10);
  mbDevice->link.modbusRtu.unitAddress = (uint8_t)strtol(  confValue( confNode, unitAddress ) , NULL, 10);
  mbDevice->link.modbusRtu.baudRate    = (uint16_t)strtol( confValue( confNode, baudRate  ) , NULL, 10);
  mbDevice->link.modbusRtu.dataBits    = (uint8_t)strtol(  confValue( confNode, dataBits    ) , NULL, 10);
  mbDevice->link.modbusRtu.stopBits    = (uint8_t)strtol(  confValue( confNode, stopBits    ) , NULL, 10);
  mbDevice->link.modbusRtu.parity      = (uint8_t)strtol(  confValue( confNode, parity      ) , NULL, 10);
  mbDevice->link.modbusRtu.handshake   = (uint8_t)strtol(  confValue( confNode, handshake   ) , NULL, 10);
  return done;
}

/**
 * @brief Load all device parameters
 *
 * @param mbDevice Save configuration here
 * @param filePath Path to device configuration file
 * @return device|NULL
 */
device *loadDeviceConf(device* mbDevice, const char* filePath){
   assert(mbDevice && filePath);
   FILE* deviceConf = fopen(filePath, "r");
   if( !deviceConf ){
#ifndef NDEBUG
      printf("Error: Can't open device configuration file\n");
#endif
      return NULL;
   }
   char keyValue[200];
   mbDevice->currConfig = (_confNode*)malloc(sizeof(_confNode));
   assert(mbDevice->currConfig);
   mbDevice->currConfig->_head = NULL;
   for( int line = 1; fgets(keyValue, sizeof(keyValue), deviceConf) != NULL; line++){
      if( IGNORE( keyValue[0] ) ) continue; /* Comment or bad format */
      char* token = strtok(keyValue, "= ");
      if( !token ) { /* Bad [ token = value ] line format */
#ifndef NDEBUG
         printf("Error: Bad key = value format at line %d\n", line);
#endif         
         return NULL;
      }
        char* value = strtok( NULL, "= " );
        if( !value ) {
#ifndef NDEBUG
          printf("Error: Bad value for key at line %d\n", line);
#endif
    return NULL;
  }
    value[ strcspn(value, "\n") ] = '\0'; /* Remove new line from value */
    mbDevice->currConfig->_head = (_confDataNode*)malloc(sizeof(_confDataNode));
    mbDevice->currConfig->_head->key = (char *)malloc(strlen(token));
    mbDevice->currConfig->_head->value = (char *)malloc(strlen(value));
    assert(mbDevice->currConfig->_head);
    assert(mbDevice->currConfig->_head->key);
    assert(mbDevice->currConfig->_head->value);
    strcpy(mbDevice->currConfig->_head->key, token);
    strcpy(mbDevice->currConfig->_head->value, value);
    mbDevice->currConfig->_head->next = mbDevice->currConfig->_head;    
  } /* File scan */
  mbDevice->currConfig->next = mbDevice->headConfig;
  mbDevice->headConfig = mbDevice->currConfig;
  deviceSetCtx(mbDevice);
  fclose(deviceConf);
  return(mbDevice);
}

/**
 * @brief  Print device configuration parameters
 * @param  dev Modbus device 
 * @return done|failure
 */
int showDeviceConf(device *dev){
  assert(dev && dev->currConfig); 
  _confDataNode *confTupleNode = dev->currConfig->_head;
  _confDataNode *saveHead = confTupleNode;
  printf("Info: Device configuration\n");
  while (confTupleNode){
    printf("%s: %s\n", confTupleNode->key, confTupleNode->value);
    confTupleNode = confTupleNode->next;
  }  
  dev->currConfig->_head = saveHead; /* Restore head  */
  return done;
};

/**
 * @brief  Find and return some device configuration key and value tuple
 * @param  cn C_onfiguration N_ode to get the value
 * @param  key Data ID to fetch the current data value
 * @return char*|NULL
 */
char *confPeekValue(const _confNode *cn, const char *key){
  _dn *dataNode = (_dn*)malloc(sizeof(_dn));
  _cdn *confTupleNode = cn->_head;
  while (cn) {
    if( strcmp(confTupleNode->key, key) == 0 ) { 
      char *v = (char*)malloc(strlen(confTupleNode->value));
      assert(v);
      strcpy(v, confTupleNode->value);
      return v; 
    }
    confTupleNode = confTupleNode->next;
  }
  return NULL;
};

/**
 * @brief  Free all dynamic memory allocated
**/
int freeDeviceConf(device *dev){
  _confNode *confNodeTmp = NULL, *confNode = dev->currConfig;
  while (confNode) {
    _confDataNode *tNodeTmp=NULL, *tNode=confNode->_head;    
    while (tNode) {
      tNodeTmp = tNode;
      tNode = tNode->next;
      free(tNodeTmp->key);
      free(tNodeTmp->value);
    }  
    confNode = confNode->next;
    free(confNodeTmp);
  };
  return done;
};

/**
 * @brief Load all registers map information of device using linked lists
 *
 * @param dev Device with registers map file path loaded on loadDeviceConf()
 * @return device|NULL
 */
device *loadDeviceMap(device *dev){
  assert(dev && dev->mapFile);
  FILE *deviceMap = fopen(dev->mapFile, "r");
  if( !deviceMap ) {
    printf("Error: Can't open device registers map file: %s", dev->mapFile);
    return NULL;
  }
  //_mbrNode *dev->currMapMbr = NULL;
  _mbrDataNode *tNode = NULL;    
  for (char _kv[100]; fgets(_kv, sizeof(_kv), deviceMap) != NULL;) { /*  Scan the entire file */
    if( ( IGNORE( _kv[0] ) ) || ( ! TOKEN_KEY_MATCH( _kv, startTag) ) ) {
         continue; /* Comments */
    }/* Found a register start tag */
   dev->currMapMbr = (_mbrNode *)malloc(sizeof(_mbrNode)); /* Create a new register */
    assert(dev->currMapMbr); 
    for( int i = 0; i < _lastTuple_; i++) {  
      if( ( fgets( _kv, sizeof(_kv), deviceMap ) == NULL ) ||
          ( IGNORE( _kv[0] ) ) ) { /*INVALID characters aren't allowed. Abort scanning... */
        printf("Error: Invalid modbus register data block: %s\n", _kv);
        return NULL;
      }
      char* token = strtok(_kv, "= ");
      if( !token ){
        printf("Error: Invalid modbus register parameter: %s\n", _kv);
        return NULL;
      }
      char *value = strtok(NULL, "= ");
      if (value == NULL) { /* possible invalid [key = value] line */
        printf("Error: Invalid modbus register parameter value: %s\n", _kv);
        return NULL;
      }
      value[strcspn(value, "\n")] = '\0'; /* Remove new line from value */
      mbrMapPop(dev, token, value);
    } 
    dev->currMapMbr->next = dev->headMap;
    dev->headMap =dev->currMapMbr;
  } /* Scan entire file for registers data blocks */
  fclose(deviceMap);
  return(dev);
} /* loadDeviceMap */

/**
 * @brief  Print device registers map information
 * @param  dev Modbus device 
 * @return done|failure
 */
int showDeviceMap(const device *dev) {
  assert(dev && dev->headMap);
  _mbrNode *mbr = dev->headMap;
  for(int i=1; mbr ; i++){
    _mbrDataNode *mbrTuple = mbr->_head;
    printf("\nInfo: REGISTER[%d]\n", i);
    while (mbrTuple){
      printf("%s:%s|", mbrTuple->key, (char *)mbrTuple->value);
      mbrTuple = mbrTuple->next;
    }
    mbr = mbr->next;
  }
  puts("\n");
  return done;
};

/**
 * @brief  Insert mbr touple into current register
 * @return done|NULL
 */
int mbrMapPop(device *mbDev, char *key, char *value){
  assert(mbDev && mbDev->currMapMbr);
  _mbrDataNode *tNode;
  tNode = (_mbrDataNode *)malloc(sizeof(tNode));
  tNode->key = (char *)malloc(strlen(key));
  tNode->value = (char *)malloc(strlen(value));
  assert( tNode && tNode->key && tNode->value );
  strcpy(tNode->key, key);
  strcpy(tNode->value, value);
  tNode->next = mbDev->currMapMbr->_head;
  mbDev->currMapMbr->_head = tNode;  
  return done;
};

/**
 * @brief  Find and return some modbus register key and value tuple
 * @param  mbr Modbus device register with all parameters tuples
 * @param  key Data tuple ID to find
 * @return _mbrTupleNode|NULL
 */
char *mbrPeekValue(const _mbrNode *cn, const char *key){
  _mbrDataNode *current = cn->_head;
  while (current) {
    if( strcmp(current->key, key) == 0 ) { 
      char *v = (char*)malloc(strlen(current->value));
      assert(v);
      strcpy(v, current->value);
      return v; 
    }
    current = current->next;
  }
  return NULL;
};

/**
 * @brief  Free all dynamic memory allocated
 * @param  dev Structure with memory to be deallocated
 * @return done|failure
 */
int freeDeviceMap(device *dev){
  _mbrNode *mbrNodeTmp = NULL, *mbrNode = dev->headMap;
  while (mbrNode) {
    _mbrDataNode *tNodeTmp=NULL, *tNode=mbrNode->_head;    
    while (tNode) {
      tNodeTmp = tNode;
      tNode = tNode->next;
      free(tNodeTmp->key);
      free(tNodeTmp->value);
    }  
   dev->currMapMbr =dev->currMapMbr->next;
    free(mbrNodeTmp);
  };
  return done;
}