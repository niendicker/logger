/**
 * @file modbusTcp.h
 * @author Marcelo Niendicker Grando
 * @date 2021/09/13
 * @brief File containing example of doxygen usage for quick reference.
 *
 * Load device configuration and registers map 
 * Do the transactions = queries to modbus slave device
 * 
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 */ 

#ifndef MODBUS_TCP
#define MODBUS_TCP

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>     //malloc()
#include <unistd.h>     //read() and write()
#include <string.h>	   //strlen

#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <netdb.h>	   //hostent

#include "slave_config_parser.h"

//Modbus specification
//https://modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf

#define TCP_ADU_MAX   ((int)260) //From 1.1b specification
#define TCP_MBAP_SIZE ((int)7)
#define TCP_FC_SIZE   ((int)1)
#define TCP_DATA_SIZE (TCP_ADU_MAX - TCP_MBAP_SIZE - TCP_FC_SIZE)

#define REPLY_EX_BYTES ((int)2) //Exceptions response bytes count

enum mbFunctionCodes{
   readCoils = 1,        //Device digital inputs
   readDiscreteInputs,   //Device analogue inputs
   readHoldingRegisters, //Device configuration parameters
   readInputRegisters    //Device measurement variables
};

//ModBus Application Protocol (MBAP)
typedef struct{
   __uint16_t transactionID; //Identification of a MODBUS Request/Response transaction.
   __uint16_t protocolID;    //0 = MODBUS protocol
   __uint16_t dataSize;      //Number of following bytes
   __uint8_t unitID;         //Identification of a remote slave connected on a serial line or on other buses.
} _tcpMBAP;

//Protocol Data Unit - Modbus TCP
enum mbTcpDataIdx{
   tcpPduAddrLsb = 0,
   tcpPduAddrMsm,
   tcpPduDataSizeLsb,
   tcpPduDataSizeMsb
}; 
typedef struct{
   uint8_t functionCode;        //Define a modbus operation. ex: 0x04 = Read Input Register
   uint8_t data[TCP_DATA_SIZE]; //Contains additional information that the server uses
                                //to take the action defined by the function code
} _tcpPDU;

//Application Data Unit - Modbus TCP 
typedef struct{
   _tcpMBAP mbap;
   _tcpPDU pdu;
} _tcpADU;

//Modbus Device context
typedef struct{
   _tcpADU adu;
   device dev;
} mbCtx;



mbCtx* mbLoadConf(const char* filePath){
   assert(filePath != NULL);

	mbCtx* ctx = (mbCtx*)malloc(sizeof(mbCtx));
   assert(ctx != NULL);

   loadDeviceConf(&ctx->dev, filePath);
   assert(strlen(ctx->dev.tag) > 0);

   return ctx;
};

mbCtx* mbLoadMap(mbCtx* ctx, const char* filePath){
   assert(ctx != NULL && filePath != NULL);

   loadDeviceMap(&ctx->dev, filePath);
   assert(ctx->dev.mapSize >= 0);

   return ctx;
};

mbCtx* mbInit(const char* mbDevConfigFile, const char* mbDevMapFile){
   mbCtx* newDev = mbLoadMap(mbLoadConf(mbDevConfigFile), mbDevMapFile);
   assert(newDev != NULL);

   //Set Modbus TCP ADU init state
   newDev->adu.mbap.transactionID = 0;
   newDev->adu.mbap.protocolID = 0; //Modbus protocol 
   newDev->adu.mbap.unitID = newDev->dev.link.modbusRtu.addr; //Serial line or subnet address
   newDev->adu.mbap.dataSize = 0;
   newDev->dev.currentMapIdx = 0;

   return newDev;
};

/**
 * @brief Receive a hostname and try to get and return the IP address
 *
 * @param hostname Network name of the slave device
 * @return Success = "192.168.1.100"; fail = NULL.
 */
char * htoip(char* hostname){
	struct in_addr **addr_list;
   struct hostent *he = gethostbyname(hostname);
	char* ip = (char*)malloc(sizeof(char)*100);
   assert(he != NULL);
   assert(ip != NULL);
	
	//Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
	addr_list = (struct in_addr **) he->h_addr_list;
	
	for(int i=0; addr_list[i] != NULL; i++)
		strcpy(ip , inet_ntoa(*addr_list[i]));
	
   return ip;
}

/**
 * @brief Connect to a modbus slave device
 *
 * @param ctx Context with meta data to stablish a connection.
 * @return Success = connectionPipe; fail = -1.
 */
int mbTcpConnect(mbCtx* ctx){
   assert(ctx != NULL);

 	//Create socket
	int connectionPipe = socket(AF_INET , SOCK_STREAM , 0);
	assert(connectionPipe != (-1));

	//Server
   //Remove new line from hostname for host_to_ip
   char* hostname = strtok(ctx->dev.link.modbusTcp.hostname, "\n");
   assert(hostname != NULL);

   char* hostAddr = htoip(hostname);
   assert(hostAddr != NULL);

   //Configure server
   struct sockaddr_in mbServer;
	mbServer.sin_family = AF_INET;
   mbServer.sin_addr.s_addr = inet_addr(hostAddr);
	mbServer.sin_port = htons( ctx->dev.link.modbusTcp.port );

	//Connect to remote server
	if (connect(connectionPipe , (struct sockaddr *)&mbServer , sizeof(mbServer)) < 0){
		//printf("Could not connect to IP: %s Port: %d\n", hostAddr, ctx->dev.link.modbusTcp.port);
		return -1;
	}
   //else
	//   printf("Connected to %s at ip: %s Port: %d \n", hostname, hostAddr, ctx->dev.link.modbusTcp.port);
	
	return connectionPipe;
};

/**
 * @brief After a query is sended to slave this function get and process the reply  
 *       
 * @param ctx Context with register map to update.
 * @param connectionPipe Socket file descriptor
 * @return Success = 0; fail = -1.
 * @warning This function closes the socket
 */
int mbGetAnswer(mbCtx* ctx, int connectionPipe){
   assert(ctx != NULL && connectionPipe > 0);
   
   char replyData[TCP_DATA_SIZE];
   memset(replyData, 0, sizeof(replyData));

   int replySize = read(connectionPipe, replyData, sizeof(replyData));
   
   if(replySize < REPLY_EX_BYTES){ 
      printError();
      return -1;
   }
   else if(replySize == REPLY_EX_BYTES){ //Exception received, probably
   
   }
   else{   
      //Maybe a correct answer, lets check...
      switch (ctx->adu.pdu.functionCode){
         case readCoils:
            //TODO
         break;
         case readDiscreteInputs:
            //TODO
         break;
         case readHoldingRegisters:
            //TODO
         break;
         case readInputRegisters:
            //Check Function code
            if((uint8_t)replyData[0] == readInputRegisters){
               //Check data size
               //uint8_t replyDataBytes = (uint8_t)replyData[1];
               if((uint8_t)replyData[1] < 2){
                  printError();
                  return -1;
               }
               else{
                  switch((uint8_t)replyData[1]){
                     case 1://8 bits value
                     //Unsigned value
                     if(ctx->dev.map[ctx->dev.currentMapIdx]->isSigned == 0){  
                        ctx->dev.map[ctx->dev.currentMapIdx]->lastValid = (uint8_t)replyData[2] & 0xFF;
                        //Scale unsigned 8 bits value
                        ctx->dev.map[ctx->dev.currentMapIdx]->lastValid /= (uint8_t)ctx->dev.map[ctx->dev.currentMapIdx]->scale;
                     }
                     else{//Signed value
                        ctx->dev.map[ctx->dev.currentMapIdx]->lastValid =  (int8_t)replyData[2] & 0xFF;
                        //Scale signed 8 bits value
                        ctx->dev.map[ctx->dev.currentMapIdx]->lastValid /= (int8_t)ctx->dev.map[ctx->dev.currentMapIdx]->scale;
                     }
                     break;
                     case 2://16 bits value
                     //Just Positive values
                     if(ctx->dev.map[ctx->dev.currentMapIdx]->isSigned == 0){  
                        ctx->dev.map[ctx->dev.currentMapIdx]->lastValid = (uint16_t)(replyData[3] & 0x00FF) || ((replyData[2] << 8) & 0xFF00);
                        //Scale the value
                        ctx->dev.map[ctx->dev.currentMapIdx]->lastValid /= (uint16_t)ctx->dev.map[ctx->dev.currentMapIdx]->scale;
                     }
                     else{
                        ctx->dev.map[ctx->dev.currentMapIdx]->lastValid = (int16_t)(replyData[3] & 0x00FF) || ((replyData[2] << 8) & 0xFF00);
                        //Scale the value
                        ctx->dev.map[ctx->dev.currentMapIdx]->lastValid /= (int16_t)ctx->dev.map[ctx->dev.currentMapIdx]->scale;
                     }
                     break;
                     default:
                        printError();
                        return -1;
                     break;
                  }
               }
            }
         break;

         default:
            printError();
            return -1;
         break;
      }
   }

   return connectionPipe;
};

/**
 * @brief Send a request for each register of a device. Get and process the reply   
 *
 * @param ctx Context with register map to update.
 * @return Success = 0; fail = -1.
 */
int mbUpdate(mbCtx* ctx){
   assert(ctx != NULL);
   int connectionPipe = mbTcpConnect(ctx);

   if(connectionPipe < 0)
      return -1;
   else{//We are connected
      //Send a request for each register.
      //TODO: Optimize subsequent registers requests
      for(ctx->dev.currentMapIdx = 0; ctx->dev.currentMapIdx <= ctx->dev.mapSize; ctx->dev.currentMapIdx++){
         //--- Save request to device context ADU
         //--- MBAP
         ctx->adu.mbap.transactionID++;
         ctx->adu.mbap.dataSize = 6; //Byte count of the following fields,
                                     //including the Unit Identifier and data fields.
         //Unit address is set on init, so dont touch here
         //--- PDU
         //Func code
         ctx->adu.pdu.functionCode = ctx->dev.map[ctx->dev.currentMapIdx]->function;
         //Register start address
         ctx->adu.pdu.data[tcpPduAddrLsb] = ctx->dev.map[ctx->dev.currentMapIdx]->address & 0xFF;
         ctx->adu.pdu.data[tcpPduAddrMsm] = (ctx->dev.map[ctx->dev.currentMapIdx]->address >> 8) & 0xFF;
         //Number of registers
         ctx->adu.pdu.data[tcpPduDataSizeLsb] = ctx->dev.map[ctx->dev.currentMapIdx]->size & 0xFF;
         ctx->adu.pdu.data[tcpPduDataSizeMsb] = (ctx->dev.map[ctx->dev.currentMapIdx]->size >> 8) & 0xFF;

         //--- Use array to send all the data in one transmission
         char tcpADU[TCP_DATA_SIZE];
         memset(tcpADU, 0, sizeof(tcpADU));
         int length = 0;

         //----- BEGIN QUERY FRAME
         //--- MBAP
         tcpADU[length++] = ctx->adu.mbap.transactionID & 0xFF;
         tcpADU[length++] = (ctx->adu.mbap.transactionID >> 8) & 0xFF;
         tcpADU[length++] = ctx->adu.mbap.protocolID & 0xFF;
         tcpADU[length++] = (ctx->adu.mbap.protocolID >> 8) & 0xFF;
         tcpADU[length++] = ctx->adu.mbap.dataSize & 0xFF;
         tcpADU[length++] = (ctx->adu.mbap.dataSize >> 8) & 0xFF;
         tcpADU[length++] = ctx->adu.mbap.unitID & 0xFF;
         tcpADU[length++] = (ctx->adu.mbap.unitID >> 8) & 0xFF;
         
         //--- FUNCTION CODE
         tcpADU[length++] = ctx->adu.pdu.functionCode;
         
         //--- DATA
         //-- Address
         tcpADU[length++] = ctx->adu.pdu.data[tcpPduAddrLsb];
         tcpADU[length++]= ctx->adu.pdu.data[tcpPduAddrMsm];
         //-- Requested registers
         tcpADU[length++]= ctx->adu.pdu.data[tcpPduDataSizeLsb];
         tcpADU[length++]= ctx->adu.pdu.data[tcpPduDataSizeMsb];
         //----- END QUERY FRAME

         //Let's go... cross your fingers
         if(write(connectionPipe, tcpADU, length) != length){
            printError();
            return -1;
         }
         else{//Now wait for answer and process it
            if(mbGetAnswer(ctx, connectionPipe) <= 0){
               printError();
               return -1;
            }
         }
      }
   }

   //Test it
   if(ctx->adu.mbap.transactionID >= 4096)
      ctx->adu.mbap.transactionID = 0;
      
   return 0;
};

#endif