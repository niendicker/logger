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

/**
 * @brief Convert/Store all "non string" values on current context for fastest access
 */
void deviceSetCtx(device *mbDevice){
  assert( mbDevice && mbDevice->config );
  _ln *config = mbDevice->config;
  mbDevice->link.modbusTcp.hostname    = salloc_init(confValue(config, hostname));
  mbDevice->link.modbusTcp.ipAddress   = salloc_init(confValue(config, ipAddress));
  mbDevice->link.protocol              = (uint16_t)strtol((char*)confValue(config, protocol   ), NULL, 10);
  mbDevice->link.modbusTcp.port        = (uint16_t)strtol((char*)confValue(config, port       ), NULL, 10);
  mbDevice->link.modbusTcp.msTimeout   = (uint16_t)strtol((char*)confValue(config, msTimeout  ), NULL, 10);
  mbDevice->link.modbusRtu.unitAddress = (uint8_t )strtol((char*)confValue(config, unitAddress), NULL, 10);
  mbDevice->link.modbusRtu.baudRate    = (uint16_t)strtol((char*)confValue(config, baudRate   ), NULL, 10);
  mbDevice->link.modbusRtu.dataBits    = (uint8_t )strtol((char*)confValue(config, dataBits   ), NULL, 10);
  mbDevice->link.modbusRtu.stopBits    = (uint8_t )strtol((char*)confValue(config, stopBits   ), NULL, 10);
  mbDevice->link.modbusRtu.parity      = (uint8_t )strtol((char*)confValue(config, parity     ), NULL, 10);
  mbDevice->link.modbusRtu.handshake   = (uint8_t )strtol((char*)confValue(config, handshake  ), NULL, 10);
  mbDevice->link.modbusRtu.msTimeout   = (uint16_t)strtol((char*)confValue(config, msTimeout  ), NULL, 10);
}

/**
 * @brief Load all device parameters
 */
device *deviceConfigure(device* mbDevice, const char* filePath){
  assert(mbDevice && filePath);
  FILE* deviceConf = fopen(filePath, "r");
  assert(deviceConf);
  char keyValue[200];
  _ln *newConfig = (_ln*)calloc(sizeof(_ln), _byte_size_);
  assert(newConfig);
  for( int line = 1; fgets(keyValue, sizeof(keyValue), deviceConf) != NULL; line++){ /* search for key = value lines on file */
    if( IGNORE( keyValue[0] ) ) continue; /* Comment or bad format */
    char* token = strtok(keyValue, "= ");
    assert(token);
    char* value = strtok( NULL, "= " );
    assert(value);
    value[ strcspn(value, "\n") ] = '\0'; /* Remove new line from value  */  
    pushData(newConfig, token, value);
  } /* File scan */
  newConfig->next = mbDevice->config;
  mbDevice->config = newConfig;
  deviceSetCtx(mbDevice);
  deviceMap(mbDevice);
  fclose(deviceConf);
  return(mbDevice);
}

/**
 * @brief  Print device configuration parameters
 */
int showDeviceConf(device *dev){
  assert(dev && dev->config); 
  _ln *config = dev->config; 
  while ( config ){
    printf("\nInfo: Device configuration \n");
    listNode(dev->config);
    config = config->next;  
  }
  return done;
};

/**
 * @brief  Free all dynamic memory allocated
**/
int freeDeviceConf(device *dev){
  assert(dev);
  _ln *ln = dev->config;
  assert(ln);
  while( ln ){
    deleteNode(ln, ln->data->key);
    ln = ln->next;
  }
  return done;
};

/**
 * @brief Load all registers map information of device using linked lists
 *
 * @param dev Device with registers map file path loaded on loadDeviceConf()
 * @return device|NULL
 */
void deviceMap(device *dev){
  assert(dev);
  FILE *deviceMap = fopen((char*)confValue(dev->config, mapFile), "r");
  if( !deviceMap ) {
    return;
  }  
  for (char _kv[100]; fgets(_kv, sizeof(_kv), deviceMap) != NULL;) { /*  Scan the entire file */
    if( ( IGNORE( _kv[0] ) ) || ( ! TOKEN_KEY_MATCH( _kv, startTag) ) ) {
         continue; /* Comments */
    }/* Found a register start tag */
    _ln *newMbr = (_ln*)calloc(sizeof(_ln), _byte_size_); /* Create a new register */
    assert(newMbr); 
    pushNode(newMbr);
    for( int i = 0; i < _lastTuple_; i++) {  
      if( ( fgets( _kv, sizeof(_kv), deviceMap ) == NULL ) ||
          ( IGNORE( _kv[0] ) ) ) { /*INVALID characters aren't allowed. Abort scanning... */
        printf("Error: Invalid modbus register data block: %s\n", _kv);
        fclose(deviceMap);
        return;
      }
      char* token = strtok(_kv, "= ");
      assert(token);
      char *value = strtok(NULL, "= ");
      assert(value);
      value[strcspn(value, "\n")] = '\0'; /* Remove new line from value */
      pushData(newMbr , token, value);
    } 
    newMbr->next = dev->mbr;
    dev->mbr = newMbr;
  } /* Scan entire file for registers data blocks */
  fclose(deviceMap);
} /* loadDeviceMap */

/**
 * @brief  Print device registers map information
 * @param  dev Modbus device 
 * @return done|failure
 */
int showDeviceMap(device *dev) {
  assert(dev);
  _ln *mbr = dev->mbr;
  assert(mbr);
  while(mbr){
    printf("\nInfo: Modbus Register\n");
    _dn *data = mbr->data;
    assert(data);
    while (data){
      char *key = (char*)data->key;
      char *value = (char*)data->value;
      printf("%s:%s|", key, value);
      data = data->next;
    }
    mbr = mbr->next;
  }
  puts("\n");
  return done;
};

/**
 * @brief  Free all dynamic memory allocated
 * @param  dev Structure with memory to be deallocated
 * @return done|failure
 */
int freeDeviceMap(device *dev){
  assert(dev);
  _ln *ln = dev->mbr;
  assert(ln);
  while( ln ){
    deleteNode(ln, ln->data->key);
    ln = ln->next;
  }
  return done;
}