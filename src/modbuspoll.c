/**
 * @file modbuspoll.c
 * @author Marcelo Niendicker Grando
 * @brief Facilities related to devices(modbus context) manipulation
 * @version 0.1
 * @date 2021-10-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "modbuspoll.h"

mbCtx **initDevices(char *configDir){
  static mbCtx *devices[_mbpoll_max_dev_];
  for (int i = 0; i < _mbpoll_max_dev_; i++){
    devices[i] = NULL;
  }
  char **configFiles = getConfigs(configDir, (char*)"conf");
  char *configFullPath = salloc_init(configDir);
  for(int i =0; configFiles[i]; i++){
    srealloc(configFullPath, strlen(configFullPath)+strlen(configFiles[i]));
    strcat(configFullPath, configFiles[i]);
    devices[i] = mbInit(configFullPath);
    free(configFullPath);
    free(configFiles[i]);
  }
  return devices;
}