
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
#include "main.h"

int main(int argc , char *argv[]) {
  if( argc != _argv_sz_ ) {
    help();
    exit(done);
  }

  mbCtx **devices;
  devices = initDevices((char*)"./dev/");
  if(!devices[0])
    exit(EXIT_FAILURE);
  mbCtx *mbDevice = devices[0];
  if( mbTcpConnect(mbDevice) == failure ) {
    exit(failure);
  }
  int pollDelay = strtol( argv[pollingTime], NULL, 10);
  pollDelay = (pollDelay < MIN_POLL_TIME) ? MIN_POLL_TIME : pollDelay;
  const int64_t pollCount = strtol( argv[pollingCount], NULL, 10);
  const uint8_t eThreshold = (uint8_t)strtol( argv[errorMax], NULL, 10);

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



int help(){
  printf("\t\tmodbusPoll v%s.%s\n", VERSION_MAJOR, VERSION_MINOR);
  FILE *helpDoc = fopen("../README", "r");
  assert(helpDoc);
  puts("");
  for( int c; (c=fgetc(helpDoc)) != EOF; printf("%c", c) );
  puts("");
  return done;
}; /* help */

