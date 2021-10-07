/**
 * @file modbuspoll.h
 * @author Marcelo Niendicker Grando 
 * @brief 
 * @version 0.1
 * @date 2021-10-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef MODBUSPOLL_H
#define MODBUSPOLL_H

#include "mbTcp.h"
#include "resources.h"

#define _max_devices_ ((uint)10)

mbCtx **initDevices(char *configDir);

#endif