
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
#include "linkedlist.h"

#define \
FOREACH_CONFIG_DATA_KEY( CONFIG_DATA_KEY )   \
        /* Polling parameters */             \
        CONFIG_DATA_KEY( pollingInterval_ms) \
        CONFIG_DATA_KEY( pollingIteractions) \
        CONFIG_DATA_KEY( pollingErrorMax   ) \
        /* Modbus device parameters */       \
        CONFIG_DATA_KEY( mapFile          )  \
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
        CONFIG_DATA_KEY( _lastConfOption_ )  

#define ENUM_CONFIG_DATA_KEY(ENUM) ENUM,
enum{
   FOREACH_CONFIG_DATA_KEY(ENUM_CONFIG_DATA_KEY)
};

#define \
FOREACH_MBR_DATA_KEY( MBR_DATA_KEY ) \
  /*Modbus Register(MBR) Details */  \
        MBR_DATA_KEY( function     ) \
        MBR_DATA_KEY( address      ) \
        MBR_DATA_KEY( size         ) \
        MBR_DATA_KEY( format       ) \
        MBR_DATA_KEY( signal       ) \
        MBR_DATA_KEY( scale        ) \
        MBR_DATA_KEY( unit         ) \
        MBR_DATA_KEY( meaning      ) \
        MBR_DATA_KEY( lastValid    ) \
        MBR_DATA_KEY( _lastTuple_  )

#define ENUM_MBR_DATA_KEY(ENUM) ENUM,
enum mbrDataKeys{
    FOREACH_MBR_DATA_KEY(ENUM_MBR_DATA_KEY)
};

#define IGNORE(C) ( (C<'A'||C>'Z')&&(C<'a'||C>'z') )
#define TOKEN_KEY_MATCH(str1,str2) ( strcmp( str1, str2 ) == 0 )

#define confValue(config,key)  ( peekValue( config, (char*)#key ) )
#define mbrValue(mbr,key) ( peekValue( mbr, (char*)#key ) )

#define startTag ("MB_REGISTER_START\n")

typedef struct __tcp{
  int32_t socket;
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
   uint16_t protocol;
   _tcp modbusTcp;
   _rtu modbusRtu;
} _link;

typedef struct __mbr { /* Modbus Registers Map single linked list dts */
  int16_t  lastValid; /* Keep the last value successfull readded from device */
  _dn *data;
  struct __mbr *next;
} _mbr;

typedef struct __dev{
  _link link;    /* Connection context data - ipAddress, hostname, unitID, ADU, PDU, ... */
  _ln *config; /* Device context configuration - ^.........^.........^......^....^......,
                  Device mbr map descriptor file path, etc... */
  _ln *mbr;
  char *txADU;
  char *rxADU;
} device;

/**
 * @brief Convert/Store all "non string" values on current context for fastest access
 *
 * @param mbDevice Context with data needed to load and update 
 *                  registers from remote server/slave modbus device 
 * @return device|NULL
 */
void deviceSetCtx(device *mbDevice);

/**
 * @brief Load all device parameters
 *
 * @param mbDevice Save configuration here
 * @param filePath Path to device configuration file
 * @return device|failure
 */
device* deviceConfigure (device* dev, const char* filePath);

/**
 * @brief  Print device configuration parameters
 * @param  dev Modbus device 
 * @return done|failure
 */
int showDeviceConf(device *dev);

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
void deviceMap(device *dev);

/**
 * @brief  Print device registers map information
 * @param  ctx Modbus device context
 * @return done|failure
 */
int showDeviceMap(device *dev);

/**
 * @brief  Free all dynamic memory allocated
 * @param  dev Structure with memory to be deallocated
 * @return done|failure
 */
int freeDeviceMap(device *dev);

#endif /* ./include/mbDevice.h */