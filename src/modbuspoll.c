
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
int help();

int main(int argc , char *argv[]) {
  if( argc != _argv_sz_ ) {
    help();
    exit(done);
  }
  mbCtx *mbDevice = mbInit(argv[configFile]);
  if( mbTcpConnect(mbDevice) == failure ) {
    mbClose(mbDevice);
    exit(EXIT_FAILURE);
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
  for (int poll = -1, commError = 0;;) { /* until communication error is below the threshold */
    poll += (pollCount > 0) ? 1 : 0; /* 0 means polling forever */
    if(poll < pollCount){
      if (mbUpdateAll(mbDevice) == failure)
        commError += eThreshold > 0 ? 1 : 0;
      else if(saveData(mbDevice, 0) == -1){ /* Just notify and keep polling */
#ifndef QUIET_OUTPUT
        printf("\nError: Can't save data \n");
#endif        
      }
      if ((commError >= eThreshold) && (eThreshold != 0)) {
#ifndef QUIET_OUTPUT 
        printf("\nError: Polling terminated due to communication error limit\n");
#endif
        break;
      }
    }
    else {
#ifndef QUIET_OUTPUT
      printf("\nInfo: Polling done. Thanks!\n");
#endif
      break;
    }
    if( ( pollCount == 0 ) || (poll < (pollCount -1)) ){
      usleep(pollDelay * 1000);
    }
  } /* polling */
  saveData(mbDevice, 1); /* Free sqlCtx */
  mbClose(mbDevice);
  exit(EXIT_SUCCESS);
}; /* main */

int help(){
  printf("\t\tmodbusPoll v%s.%s\n", VERSION_MAJOR, VERSION_MINOR);
  return done;
}; /* help */

