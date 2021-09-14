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

#include <assert.h>

#include "slave_config_parser.h"

//Device options
#define MAX_OPT_IDX ((int)11) 
#define FOREACH_CONFIG_OPTION(CONFIG_OPTION) \
           CONFIG_OPTION(slaveName)          \
           CONFIG_OPTION(protocol)           \
           CONFIG_OPTION(slaveTcpHostname)   \
           CONFIG_OPTION(slaveTcpAddr)       \
           CONFIG_OPTION(slaveTcpPort)       \
           CONFIG_OPTION(slaveRtuAddr)       \
           CONFIG_OPTION(ttyBaudRate)        \
           CONFIG_OPTION(ttyDataBits)        \
           CONFIG_OPTION(ttyStopBits)        \
           CONFIG_OPTION(ttyParity)          \
           CONFIG_OPTION(ttyHandshake)

#define GENERATE_ENUM_OPTIONS(ENUM) ENUM,
enum optIdx{
    FOREACH_CONFIG_OPTION(GENERATE_ENUM_OPTIONS)
};

#define GENERATE_STRING_OPTIONS(STRING) #STRING,
static const char *config_options[] = {
    FOREACH_CONFIG_OPTION(GENERATE_STRING_OPTIONS)
};

//Modbus register data fields
#define MB_REG_TUPLES 8
#define FOREACH(REGISTER_FIELD)     \
           REGISTER_FIELD(function) \
           REGISTER_FIELD(address)  \
           REGISTER_FIELD(size)     \
           REGISTER_FIELD(format)   \
           REGISTER_FIELD(signal)   \
           REGISTER_FIELD(scale)    \
           REGISTER_FIELD(unit)     \
           REGISTER_FIELD(meaning)

#define GENERATE_ENUM_REG_TUPLES(ENUM) ENUM,
enum registerMapIdx{
    FOREACH(GENERATE_ENUM_REG_TUPLES)
};

#define GENERATE_STRING_REG_TUPLES(STRING) #STRING,
static const char *registerMap[] = {
    FOREACH(GENERATE_STRING_REG_TUPLES)
};

#define IGNORE(C) ( (C<'A'||C>'Z') && (C<'a'||C>'z') || (C == '\n')  )

//Create a new [device] data structure and load the values from [filePath] config file 
device* loadDeviceConf(device* mbSlave, const char* filePath){
   assert(mbSlave != NULL && filePath != NULL);
   FILE* deviceConf = fopen(filePath, "r");
   assert(deviceConf != NULL);
   
   //device* mbSlave = (device*)malloc(sizeof(device));
   //assert(mbSlave != NULL);

   char keyValue[200];
   memset(keyValue, 0, sizeof(keyValue));

   while (fgets(keyValue, sizeof(keyValue), deviceConf) != NULL){
      if(IGNORE(keyValue[0]))
         continue; //Comment or bad format
      else{
         char* token = strtok(keyValue, "= "); //Get the key
         assert(token != NULL);
         //printf("%s=", token);
         char* value = strtok(NULL, "= "); //Get the value
         assert(value != NULL);
         //printf("%s", value);

         //Find and load each configuration parameter
         for(int tokenId=0; tokenId < MAX_OPT_IDX; tokenId++){
            if(strcmp(token, config_options[tokenId]) == 0){ //We found!
               switch (tokenId) { //Quat we found?
                  case slaveName:
                     strcpy(mbSlave->tag, value);
                  break;
                  case protocol:
                     strcpy(mbSlave->link.protocol, value);
                  break;
                  case slaveTcpHostname:
                     strcpy(mbSlave->link.modbusTcp.hostname, value);
                  break;
                  case slaveTcpAddr:
                     strcpy(mbSlave->link.modbusTcp.addr, value);
                  break;
                  case slaveTcpPort:
                     mbSlave->link.modbusTcp.port =  strtol(value, NULL, 10);
                  break;
                  //TODO: RTU config
                  default:
                     
                  break;
               }
            }
         }
      }
   }
   fclose(deviceConf);
   return(mbSlave);
}

//Allocate memory and store each modbus register data found in deviceMap file
device* loadDeviceMap(device* dev, const char *filePath){
   assert((filePath != NULL) && (dev != NULL));

   FILE* deviceMap = fopen(filePath, "r");
   assert(deviceMap != NULL);
   
   char keyValue[200];
   memset(keyValue, 0, sizeof(keyValue));

   const char startTag[] = "register+++\n";

   int registersCount = dev->mapSize = -1; //Keep -1 so the first ++ gives index 0

   while(fgets(keyValue, sizeof(keyValue), deviceMap) != NULL){      
      if(IGNORE(keyValue[0]))   
         continue; //Comment or bad format

      if(strcmp(keyValue, startTag) == 0){ //Init modbus register description data
         //registersCount++;
         
         dev->mapSize = ++registersCount;
         //printf("\nRegister[%d]:\n", registersCount);
         dev->map[registersCount] = (mbRegister*)malloc(sizeof(mbRegister));
         assert(dev->map[registersCount] != NULL);         
         memset(dev->map[registersCount], 0, sizeof(mbRegister));
         //dev->map[registersCount]->lastValid = 0;

         while(1){            
            if(fgets(keyValue, sizeof(keyValue), deviceMap) == NULL)
               return dev;

            if(IGNORE(keyValue[0])) //End of register block data
               break;
 
            char* token = strtok(keyValue, "= "); //Get the key
            assert(token != NULL);
            //printf("%s=", token); 
            //Verify if is a valid token
            for(int tokenID=0; tokenID < MB_REG_TUPLES; tokenID++){
               if(strcmp(token, registerMap[tokenID]) == 0){ //Modbus Register property token found
                  
                  char* rawValue = strtok(NULL, "= "); //Get the value
                  assert(rawValue != NULL);
                  //printf("%s", rawValue);

                  int intValue = strtol(rawValue, NULL, 10);
                  
                  switch (tokenID){ //Save data to internal data structure
                     case function:
                        dev->map[registersCount]->function = intValue; 
                     break;
                     case address:
                        dev->map[registersCount]->address = intValue;
                     break;
                     case size:
                        dev->map[registersCount]->size = intValue;
                     break;
                     case format:
                        strcpy(dev->map[registersCount]->format, rawValue);
                     break;
                     case signal:
                        dev->map[registersCount]->isSigned = intValue;
                     break;
                     case scale:
                        dev->map[registersCount]->scale = intValue;
                     break;
                     case unit:
                        strcpy(dev->map[registersCount]->unit, rawValue);
                     break;
                     case meaning:
                        strcpy(dev->map[registersCount]->meaning, rawValue);
                     break;
                     default:
                     break;
                  }
               }
            }
         }
      }  
   }
   
   if(fileno(deviceMap) >= 0) 
      fclose(deviceMap);
   return(dev);
}