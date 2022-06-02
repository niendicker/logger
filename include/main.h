
/**
 * @file   logger.h
 * @author Marcelo Niendicker Grando
 * @date   2021/09/13
 * @brief  Modbus polling tool
 */ 
#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include "version.h"
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

