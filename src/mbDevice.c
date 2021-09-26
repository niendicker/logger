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

int deviceSetCtx(device *mbDevice){
  assert( mbDevice && mbDevice->_currConfig );
  _config *currConfing = mbDevice->_currConfig;
  strcpy ( mbDevice->link.protocol, confValue(currConfing, protocol) );
  strcpy ( mbDevice->link.modbusTcp.ipAddress, confValue(currConfing, ipAddress) );
  strcpy ( mbDevice->link.modbusTcp.hostname, confValue(currConfing, hostname) );
  mbDevice->link.modbusTcp.port        = (uint16_t)strtol( confValue( currConfing, port      ) , NULL, 10);
  mbDevice->link.modbusTcp.msTimeout   = (uint16_t)strtol( confValue( currConfing, msTimeout ) , NULL, 10);
  mbDevice->link.modbusRtu.unitAddress = (uint8_t)strtol(  confValue( currConfing, unitAddress ) , NULL, 10);
  mbDevice->link.modbusRtu.baudRate    = (uint16_t)strtol( confValue( currConfing, baudRate  ) , NULL, 10);
  mbDevice->link.modbusRtu.dataBits    = (uint8_t)strtol(  confValue( currConfing, dataBits    ) , NULL, 10);
  mbDevice->link.modbusRtu.stopBits    = (uint8_t)strtol(  confValue( currConfing, stopBits    ) , NULL, 10);
  mbDevice->link.modbusRtu.parity      = (uint8_t)strtol(  confValue( currConfing, parity      ) , NULL, 10);
  mbDevice->link.modbusRtu.handshake   = (uint8_t)strtol(  confValue( currConfing, handshake   ) , NULL, 10);
  return done;
}

/**
 * @brief Load all device parameters
 *
 * @param mbDevice Save configuration here
 * @param filePath Path to device configuration file
 * @return device|NULL
 */
device *deviceConfig(device* mbDevice, const char* filePath){
  assert(mbDevice && filePath);
  FILE* deviceConf = fopen(filePath, "r");
  if( !deviceConf ){
#ifndef NDEBUG
    printf("Error: Can't open device configuration file\n");
#endif
    return NULL;
   }
  char keyValue[200];
  mbDevice->_currConfig = (_config*)malloc(sizeof(_config*));
  assert(mbDevice->_currConfig);
  mbDevice->_currConfig->_next = NULL;
  for( int line = 1; fgets(keyValue, sizeof(keyValue), deviceConf) != NULL; line++){
    if( IGNORE( keyValue[0] ) ) continue; /* Comment or bad format */
    char* token = strtok(keyValue, "= ");
    if( token == NULL ) { /* Bad [ token = value ] line format */
      return NULL;
    }
    char* value = strtok( NULL, "= " );
    if( value == NULL ) {
      return NULL;
    }
    value[ strcspn(value, "\n") ] = '\0'; /* Remove new line from value */  
    mbDevice->_currConfig = (_config*)malloc(sizeof(_config*));
    mbDevice->_currConfig->_head->_key = (char *)malloc(strlen(token));
    mbDevice->_currConfig->_head->_value = (char *)malloc(strlen(value));
    assert(mbDevice->_currConfig->_head);
    assert(mbDevice->_currConfig->_head->_key);
    assert(mbDevice->_currConfig->_head->_value);
    strcpy(mbDevice->_currConfig->_head->_key, token);
    strcpy(mbDevice->_currConfig->_head->_value, value);
    mbDevice->_currConfig->_head->_next = mbDevice->_currConfig->_head;    
  } /* File scan */
  mbDevice->_currConfig->_next = mbDevice->_currConfig;
  mbDevice->_headConfig = mbDevice->_currConfig;
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
  assert(dev && dev->_currConfig); 
  printf("Info: Device configuration\n");
  while (dev->_currConfig->_curr){
    char * key = (char*)dev->_currConfig->_curr->_key;
    char * value = (char*)dev->_currConfig->_curr->_value;
    printf("%s: %s\n", key, value);
    dev->_currConfig->_curr = dev->_currConfig->_curr->_next;
  }  
  dev->_currConfig->_curr = dev->_currConfig->_head; /* Restore head  */
  return done;
};

/**
 * @brief  Find and return some device configuration key and value tuple
 * @param  cn C_onfiguration N_ode to get the value
 * @param  key Data ID to fetch the current data value
 * @return char*|NULL
 */
char *confPeekValue(_config *config, char *key){
  assert(config && key);
  while (config->_curr) {
    if( strcmp(config->_curr->_key, key) == 0 ) { 
      char *v = (char*)malloc(strlen(config->_curr->_value));
      assert(v);
      strcpy(v, config->_curr->_value);
      config->_curr = config->_head;
      return v; 
    }
    config->_curr = config->_curr->_next;
  }
  config->_curr = config->_head;
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
  FILE *deviceMap = fopen(confValue(dev->_currConfig, mapFile), "r");
  if( !deviceMap ) {
    printf("Error: Can't open device registers map file");
    return NULL;
  }  
  for (char _kv[100]; fgets(_kv, sizeof(_kv), deviceMap) != NULL;) { /*  Scan the entire file */
    if( ( IGNORE( _kv[0] ) ) || ( ! TOKEN_KEY_MATCH( _kv, startTag) ) ) {
         continue; /* Comments */
    }/* Found a register start tag */
    dev->_currMbr = (_mbr*)malloc(sizeof(_mbr*)); /* Create a new register */
    assert(dev->_currMbr); 
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
    dev->_currMbr->_next = dev->_currMbr;
    dev->_headMbr = dev->_currMbr;
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
  while(dev->_currMbr){
    printf("\nInfo: Modbus Register\n");
    while (dev->_currMbr->_curr){
      char * key = (char*)dev->_currMbr->_curr->_key;
      char * value = (char*)dev->_currMbr->_curr->_value;
      printf("%s:%s|", key, value);
      dev->_currMbr->_curr = dev->_currMbr->_curr->_next;
    }
    dev->_currMbr = dev->_currMbr->_next;
  }
  puts("\n");
  return done;
};

/**
 * @brief  Insert mbr touple into current register
 * @return done|NULL
 */
int mbrMapPop(device *dev, char *key, char *value){
  assert(dev && key && value);
  _dn *dn = (_dn*)malloc(sizeof(_dn*));
  assert(dn);
  dn->_key = (char*)malloc(strlen(key));
  assert(dn->_key);
  dn->_value = (char*)malloc(strlen(value));
  assert(dn->_value);
  strcpy(dn->_key, key);
  strcpy(dn->_value, value);
  dn->_next = dev->_currMbr->_head;
  dev->_currMbr->_head = dn;
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
  while (mbr->_curr) {
    if( strcmp(mbr->_curr->_key, key) == 0 ) { 
      char *v = (char*)malloc(strlen(mbr->_curr->_value));
      assert(v);
      strcpy(v, mbr->_curr->_value);
      mbr->_curr = mbr->_head;
      return v; 
    }
    mbr->_curr = mbr->_curr->_next;
  }
  mbr->_curr = mbr->_head;
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