#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


#define FOREACH_CONFIG_OPTION(CONFIG_OPTION) \
           CONFIG_OPTION(slaveName=0)        \
           CONFIG_OPTION(protocol)           \
           CONFIG_OPTION(slaveTcpHostname)   \
           CONFIG_OPTION(slaveTcpAddr)       \
           CONFIG_OPTION(slaveTcpPort)       \
           CONFIG_OPTION(slaveRtuAddr)       \
           CONFIG_OPTION(ttyBaudRate)        \
           CONFIG_OPTION(ttyDataBits)        \
           CONFIG_OPTION(ttyStopBits)        \
           CONFIG_OPTION(ttyParity)          \
           CONFIG_OPTION(ttyHandshake)       \
           CONFIG_OPTION(mapFile)            \
           CONFIG_OPTION(lastParameter)      //This is the magic tag. 
                                             //Include New params above ^^^

#define GENERATE_ENUM_OPTIONS(ENUM) ENUM,
//struct _devOpt{
//   int optIdx; 
   enum opt{
      FOREACH_CONFIG_OPTION(GENERATE_ENUM_OPTIONS)
   };
//};

int main(int argc, char* argv[]){
   for(int i=0; i < lastParameter; i++){
      printf("%d", i);
   }
   return 0;
}