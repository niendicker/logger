
/**
 * @file   main.c
 * @author Marcelo Niendicker Grando
 * @date   2021/09/13
 * @brief  Modbus polling tool
 *  
 * @see Modbus specification
 * @see https://modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
 * @see https://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf
 */ 
#ifndef MODBUSPOLL_H
#define MODBUSPOLL_H

#define VERSION_MAJOR "00"
#define VERSION_MINOR "00"
#include <stdbool.h>
#include "error.h"
#include "mbTcp.h"
#include "postgresql.h"

#define _min_poll_time_ ((int)1) //ms
#define _max_devices_ ((int)10)

enum argvIndex {
  myself=0, 
  configFile,
  _argv_sz_
};

enum argConstraints{
  argMin=2,
  argMax=( _argv_sz_ )
};

#endif/* main */

