
/**
 * @file   device_config_parser.h
 * @author Marcelo Niendicker Grando
 * @date   2021/09/13
 * @brief  Load device configuration and registers map information 
 * 
 * @see Modbus specification
 * @see https://modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
 * @see https://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf
 */ 

#ifndef MB_DEVICE
#define MB_DEVICE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "errorCtrl.h"
// /#include "mbTcp.h"

#define   FOREACH_CONFIG_DATA_KEY( CONFIG_DATA_KEY )  \
            CONFIG_DATA_KEY( tag              )  \
            CONFIG_DATA_KEY( protocol         )  \
            CONFIG_DATA_KEY( hostname         )  \
            CONFIG_DATA_KEY( ipAddress        )  \
            CONFIG_DATA_KEY( port             )  \
            CONFIG_DATA_KEY( msTimeout        )  \
            CONFIG_DATA_KEY( unitAddress      )  \
            CONFIG_DATA_KEY( baudRate         )  \
            CONFIG_DATA_KEY( dataBits         )  \
            CONFIG_DATA_KEY( stopBits         )  \
            CONFIG_DATA_KEY( parity           )  \
            CONFIG_DATA_KEY( handshake        )  \
            CONFIG_DATA_KEY( mapFile          )  \
            CONFIG_DATA_KEY( _lastConfOption_ )  //Include New params above ^^^

#define ENUM_CONFIG_DATA_KEY(ENUM) ENUM,
enum{
   FOREACH_CONFIG_DATA_KEY(ENUM_CONFIG_DATA_KEY)
};

#define   FOREACH_MBR_DATA_KEY( DATA_KEY ) \
            DATA_KEY( function     ) \
            DATA_KEY( address      ) \
            DATA_KEY( size         ) \
            DATA_KEY( format       ) \
            DATA_KEY( signal       ) \
            DATA_KEY( scale        ) \
            DATA_KEY( unit         ) \
            DATA_KEY( meaning      ) \
            DATA_KEY( _lastTuple_  ) //Include New tuples above ^^^

#define ENUM_MBR_DATA_KEY(ENUM) ENUM,
enum mbrDataKeys{
    FOREACH_MBR_DATA_KEY(ENUM_MBR_DATA_KEY)
};

#define IGNORE(C) ( ((C<'A'||C>'Z') && (C<'a'||C>'z')) || (C == '\n')  )
#define TOKEN_KEY_MATCH(str1,str2) ( strcmp( str1, str2 ) == 0 )

#define confValue(cNode,key)  ( confPeekValue( cNode,#key ) )
#define mbrValue(mbrNode,key) ( mbrPeekValue( mbrNode,#key ) )

#define startTag ("MB_REGISTER_START\n")

typedef struct __tcp{
  uint32_t socket;
  uint16_t port;
  uint8_t msTimeout; /* Maximum waiting time for any communication reply in ms */
  char *hostname;
  char *ipAddress;
} _tcp;

typedef struct __rtu{
  u_int8_t unitAddress, dataBits, stopBits, parity, handshake;
  u_int16_t baudRate;
  uint8_t msTimeout; /* Maximum waiting time for any communication reply in ms */
} _rtu;

typedef struct __link{
   char* protocol;
   _tcp modbusTcp;
   _rtu modbusRtu;
} _link;

typedef struct __dn{  /* Generic data node for single linked list */
  void *_key;
  void *_value;
  struct __dn  *_next;
} _dn; 

typedef struct __config { /* Configurations single linked list dts */
  _dn *_head;
  _dn *_curr;
  struct __config *_next;
} _config;

typedef struct __mbr { /* Modbus Registers Map single linked list dts */
  int16_t  lastValid; /* Keep the last value successfull readded from device */
  _dn *_head;
  _dn *_curr;
  struct __mbr *_next;
} _mbr;

typedef struct __dev{
  _link link;    /* Data to connect to device */
  _config *_headConfig;
  _config *_currConfig;
  _mbr *_headMbr;
  _mbr *_currMbr;
  char *txADU;
  char *rxADU;
} device;

/**
 * @brief Load all device parameters
 *
 * @param mbDevice Save configuration here
 * @param filePath Path to device configuration file
 * @return device|failure
 */
device* deviceConfig(device* dev, const char* filePath);

/**
 * @brief  Print device configuration parameters
 * @param  dev Modbus device 
 * @return done|failure
 */
int showDeviceConf(device *dev);

/**
 * @brief  Find and return some device configuration key and value tuple
 * @param  conf Configuration to get the value
 * @param  key Data tuple ID to find
 * @return _confTupleNode|NULL
 */
char *confPeekValue(_config *config, char *key);

/**
 * @brief  Free all dynamic memory allocated
 * @param  dev Structure with memory to be deallocated
 * @return done|failure
 */
int freeDeviceConf(device *dev);

/**
 * @brief Load all registers map information of device
 *
 * @param dev Device with registers map file path loaded on loadDeviceConf()
 * @return device|failure
 */
device *deviceMap(device *dev);

/**
 * @brief  Print device registers map information
 * @param  ctx Modbus device context
 * @return done|failure
 */
int showDeviceMap(device *dev);

/**
 * @brief  Insert mbr touple into current register
 * @return done|NULL
 */
int mbrMapPop(device *dev, char *key, char *value);

/**
 * @brief  Find and return some modbus register key and value tuple
 * @param  mbr Modbus device register with all parameters tuples
 * @param  key Data tuple ID to find
 * @return value|NULL
 */
char *mbrPeekValue(_mbr *mbr, char *key);

/**
 * @brief  Free all dynamic memory allocated
 * @param  dev Structure with memory to be deallocated
 * @return done|failure
 */
int freeDeviceMap(device *dev);

#endif /* ./include/mbDevice.h */