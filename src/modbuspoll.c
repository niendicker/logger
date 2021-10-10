
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
#include "modbuspoll.h"

int main(int argc , char *argv[]) {
  if( argc != _argv_sz_ ) {
    help();
    exit(done);
  }

  mbCtx **devices;
  char *dir = salloc_init(argv[configDir]);
  devices = initDevices(dir);
  free(dir);
  if(!devices[0])
    exit(EXIT_FAILURE);
  mbCtx *mbDevice = devices[0];
  if( mbTcpConnect(mbDevice) == failure ) {
    exit(failure);
  }
  char *pInterval = salloc_init(confValue(mbDevice->dev.config, pollingInterval_ms));
  char *pCout = salloc_init(confValue(mbDevice->dev.config, pollingIteractions));
  char *pErrorMax = salloc_init(confValue(mbDevice->dev.config, pollingErrorMax));
  int pollDelay = (int)strtol( pInterval, NULL, 10);
  int pollCount = (int)strtol( pCout, NULL, 10);
  int eThreshold = (int)strtol( pErrorMax, NULL, 10);
  pollDelay = (pollDelay < _min_poll_time_) ? _min_poll_time_ : pollDelay;
  pollCount = (pollCount < 0) ? 0 : pollCount;
  eThreshold = (eThreshold < 0) ? 0 : eThreshold;
  free(pErrorMax);
  free(pCout);
  free(pInterval);
  for (int poll = -1, eCount = 0;;) { /* until communication error is below the threshold received from argv[3] */
    poll += (pollCount > 0) ? 1 : 0; /* 0 means polling forever */
    if(poll < pollCount){
      if (mbUpdate(mbDevice) == failure)
        eCount += eThreshold > 0 ? 1 : 0;
      if(saveData(mbDevice) == -1){ /* Just notify and keep polling */
#ifndef QUIET_OUTPUT
      printf("\nError: Can't save data \n");
#endif        
      }
    }
    else {
#ifndef QUIET_OUTPUT
      printf("\nInfo: Polling terminated due to argument polling count\n");
#endif
      break;
    }
    if (eCount >= (eThreshold+1)) {
#ifndef QUIET_OUTPUT 
      printf("\nError: Polling terminated due to communication error limit\n");
#endif
      break;
    }
    if( ( pollCount == 0 ) || (poll < (pollCount -1)) ){
      usleep(pollDelay * 1000);
    }
  } /* polling */
  mbClose(mbDevice);
  exit(EXIT_SUCCESS);
}; /* main */

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

int help(){
  printf("\t\tmodbusPoll v%s.%s\n", VERSION_MAJOR, VERSION_MINOR);
  FILE *helpDoc = fopen("../README", "r");
  assert(helpDoc);
  puts("");
  for( int c; (c=fgetc(helpDoc)) != EOF; printf("%c", c) );
  puts("");
  fclose(helpDoc);
  return done;
}; /* help */

