
/**
 * @file modbusTcp.h
 * @author Marcelo Niendicker Grando
 * @date 2021/09/13
 * @brief File containing example of doxygen usage for quick reference.
 *
 * Load device configuration and registers map 
 * 
 * 
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 */ 

#ifndef device_CONFIG_PARSER
#define device_CONFIG_PARSER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct{
   char hostname[64], addr[20];
   int port;
} tcp;

typedef struct{
   u_int8_t addr, dataBits, stopBits, parity;
   u_int16_t baudRate;
} rtu;

typedef struct{
   char protocol[20];
   tcp modbusTcp;
   rtu modbusRtu;
} connection;

typedef struct{
   uint8_t  function;
   uint16_t address;
   uint8_t  size;
   char     format[10];
   uint8_t  isSigned;
   int      scale;
   char     unit[10];
   char     meaning[32];
   int16_t  lastValid; //Keep the last value successfull readded from device
} mbRegister;

typedef struct{
   char tag[32];      //Just a tag for the device
   connection link;   //Data to connect to device
   int currentMapIdx; //The register that is being manipulated right now
   int mapSize;       //How many registers has the map?
   mbRegister* map[]; //Registers
} device;

device* loadDeviceConf(device* mbDevice, const char* filePath);
device* loadDeviceMap(device* dev, const char *filePath);

#endif