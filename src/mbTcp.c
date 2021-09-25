/**
 * @file   modbusTcp.h
 * @author Marcelo Niendicker Grando
 * @date   2021/09/17
 *
 * @brief  @see mbTcp.h
 *
 * @see Modbus specification
 * @see https://modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
 * @see https://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf
 * @see https://www.ni.com/pt-br/innovations/white-papers/14/the-modbus-protocol-in-depth.html
 */

#include "mbTcp.h"

#define LTOS_LSB(LONG) (LONG & 0xFF)
#define LTOS_MSB(LONG) ((LONG >> 8) & 0xFF)
#define STOL(S_MSB, S_LSB) (((S_MSB << 8) & 0xFF) || (S_LSB & 0xFF))

/**
 * @brief  Print device configuration
**/
int mbShowConf(const mbCtx *ctx) {
   showDeviceConf(&ctx->dev);
   return done;
};

/**
 * @brief  Print device registers map information
**/
int mbShowRegistersMap(const mbCtx *ctx) {
  showDeviceMap(&ctx->dev);
  return done;
};

/**
 * @brief Create a new context.
 *        Load all device parameters from filePath file
**/
mbCtx * mbLoadConf(const char * filePath) {
  assert(filePath);
  mbCtx * ctx = (mbCtx * ) malloc(sizeof(mbCtx));
  assert(ctx);
  if ( !loadDeviceConf( & ctx->dev, filePath) ) { return NULL; }
  return ctx;
};

/**
 * @brief Load context device's modbus registers map (MBRM)
**/
mbCtx *mbLoadMap(mbCtx *ctx) {
  assert(ctx);
  if ( !loadDeviceMap(&ctx->dev) ) { return NULL; }
  return ctx;
};

/**
 * @brief Load context device parameters from filePath file
 *        Load context device registers map (MBR) metadata
 *        Initialize context
**/
mbCtx * mbInit(const char * mbDevConfigFile) {
  assert(mbDevConfigFile);
  mbCtx *newDev = mbLoadConf(mbDevConfigFile);
  if ( !newDev ) { return NULL; }
  if ( !mbLoadMap(newDev) ) { return NULL; }
  newDev->adu.mbap.tID = 0;
  newDev->adu.mbap.pID = 0; //Modbus protocol
  newDev->adu.mbap.uID = newDev -> dev.link.modbusRtu.addr; //Serial line or subnet address
  newDev->adu.mbap.fBytes = 0;
  newDev->dev.link.modbusTcp.socket = failure;
  newDev->dev.link.modbusTcp.msTimeout = (uint8_t)100; /* milli seconds */
  newDev->dev.link.modbusRtu.msTimeout = (uint8_t)100;
#ifndef NDEBUG
  mbShowConf(newDev);    
  mbShowRegistersMap(newDev);
#endif    
  return newDev;
};

/**
 * @brief Open a socket, Resolve device Hostname and Connect to IP address
 * @param ctx Context with meta data to stablish a connection
 * @return done|failure
 */
int mbTcpConnect(mbCtx *ctx) {
  assert(ctx && (ctx->dev.link.modbusTcp.socket == failure));
  ctx->dev.link.modbusTcp.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  assert(ctx->dev.link.modbusTcp.socket);
  char *ip = confValue(ctx->dev.currConfig, slaveTcpAddr);
  assert(ip);
  char *hostAddr;
  if(strlen(ip) != (sizeof("MOD.BUS._PO.LL_")-1)){ /* if IP address is configured don't perform DNS translation */
    char *hostname = confValue(ctx->dev.currConfig, slaveTcpHostname);
    assert(hostname);
    hostAddr = htoip(hostname); /* DNS translation. Resolve hostname => IP address */
    assert(hostAddr);
  }
  else{
    hostAddr = ip;
  }
  struct sockaddr_in mbServer; /* Set connection parameters */
  mbServer.sin_addr.s_addr = inet_addr(hostAddr);
  mbServer.sin_family = AF_INET;  
  mbServer.sin_port = htons(ctx->dev.link.modbusTcp.port);
#ifndef NDEBUG
  printf("Info: Connecting to %s @%s:%d \n", ctx->dev.tag, hostAddr, ctx->dev.link.modbusTcp.port);
#endif
  if (connect(ctx->dev.link.modbusTcp.socket, (struct sockaddr *)&mbServer, sizeof(mbServer)) == failure) {
#ifndef NDEBUG
    printf("Error: Connection refused from %s \n", ctx->dev.tag);
#endif
    return failure;
  } 
  //else {
  //  strcpy(ipAddress, hostAddr); /* If we could connect using hostname */
  //                               /* overwrite IP parameter for current instance */
#ifndef NDEBUG
  printf("Info: Connected \n");
#endif
  return done;
};

/**
 * @brief Close socket connection
 */
int mbTcpDisconnect(mbCtx * ctx) {
  assert(ctx && ctx->dev.link.modbusTcp.socket);
  close(ctx->dev.link.modbusTcp.socket);
  ctx->dev.link.modbusTcp.socket = failure;
#ifndef NDEBUG
  printf("Info: Socket closed \n");
#endif
  return done;
};

/**
 * @brief Free dynamic allocated memory 
**/
int mbClose(mbCtx *ctx){
  mbTcpDisconnect(ctx);
  freeDeviceMap(&ctx->dev);
  freeDeviceConf(&ctx->dev);
  ctx->adu.mbap.tID = 0;
  ctx->adu.mbap.pID = 0; //Modbus protocol
  ctx->adu.mbap.uID = 0; //Serial line or subnet address
  ctx->adu.mbap.fBytes = 0;
  ctx->dev.link.modbusTcp.socket = failure;
  ctx->dev.link.modbusTcp.msTimeout = (uint8_t)100; /* milli seconds */
  ctx->dev.link.modbusRtu.msTimeout = (uint8_t)100;
  ctx = NULL;
  return done;
};

/**
 * @brief Close and reopen socket connection 
**/
int mbTcpReconnect(mbCtx *ctx){
  assert(ctx && ctx->dev.link.modbusTcp.socket);
  mbTcpDisconnect(ctx);
  mbTcpConnect(ctx);
  return done;
};

/**
 * @brief Receive a hostname and try to get and return the IP address
 * @param hostname Network name of the slave device
 * @return ipAddress|NULL.
 */
char *htoip(char *hostname) {
  struct hostent *he = gethostbyname(hostname);
  assert(he);
  struct in_addr **addr_list = (struct in_addr**)he->h_addr_list; /* Cast since h_addr_list also has the ip address in long format only */
  char *ip = (char*)malloc( sizeof(char) * 100 );
  assert(ip);
  for (int i = 0; addr_list[i] != NULL; i++) {
    strcpy(ip, inet_ntoa(*addr_list[i])); /* Return the first one; */
  }
#ifndef NDEBUG
  printf("Info: DNS translation finished for %s to IP %s \n", hostname, ip);
#endif
  return ip;
};

/**
 * @brief Print to stdout sendded ADU raw data
**/
int _mbRequestRaw(const mbCtx *ctx){
  assert(ctx);
  printf("Info: Sended  data  : ");
  for (size_t i = 0; i < _adu_size_; i++){
    uint8_t data = (uint8_t)ctx->adu.pdu.data[i];
    printf("%02X", data); /* Hex value */
  }
  printf("\n");
  return done;
};

/**
 * @brief Print to stdout received ADU raw data
**/
int _mbReplyRaw(const mbCtx *ctx){
  assert(ctx);
  printf("Info: Received data : ");
  for (size_t i = 0; i < _adu_size_; i++){
    uint8_t data = (uint8_t)ctx->adu.pdu.data[i];
    printf("%02X", data); /* Hex value */
  }
  printf("\n");
  return done;
};

/**
 * @brief When a correct modbus reply data size is received
 *        this function parse and save received data to ctx  
**/
int mbParseReply(mbCtx *ctx, _mbrNode *mbr){
  assert(ctx);
  uint16_t tID, pID, size;
  uint8_t uID, fCD, plSz;
  tID  = (uint16_t)STOL(ctx->adu.pdu.data[tIDMsb],ctx->adu.pdu.data[tIDLsb]); /* MBAP */
  pID  = (uint16_t)STOL(ctx->adu.pdu.data[pIDMsb],ctx->adu.pdu.data[pIDLsb]); 
  size = (uint16_t)STOL(ctx->adu.pdu.data[fBytesMsb],ctx->adu.pdu.data[fBytesLsb]);
  uID  = (uint8_t)ctx->adu.pdu.data[uID]; 
  fCD  = (uint8_t)ctx->adu.pdu.data[functionCode]; /* PDU */ 
  plSz = (uint8_t)ctx->adu.pdu.data[replySz]; /* Reply payload size */ 
  if(tID != ctx->adu.mbap.tID){  /* Transaction ID need to be the same for query/reply */
#ifndef NDEBUG
    printf("Error: Transaction ID \n\n");
#endif    
    return failure;
  }
  if(pID != ctx->adu.mbap.pID){     /* Standard 0 = modbus tcp  */
#ifndef NDEBUG
    printf("Error: pID \n\n");
#endif    
    return failure;
  }
  if(uID != ctx->dev.link.modbusRtu.unitAddress){ /* Unit ID is the modbus RTU (Serial comm.) address */
#ifndef NDEBUG
    printf("Error: Unit ID \n\n");
#endif    
    return failure;
  }
  if(size < 4 ){ /* At least 1B(uID) + 1B(fCode) + 1B(payload size) + 1B of payload */
#ifndef NDEBUG
    printf("Error: Data size to short \n\n");
#endif    
    return failure;
  }
  uint8_t fcdRequest = ctx->adu.pdu.functionCode; 
  if(fCD != fcdRequest){
#ifndef NDEBUG
    printf("Error: Function code \n\n");
#endif   
    return failure;
  }
  if(plSz != 2){
#ifndef NDEBUG
    printf("Error: Payload size \n\n");
#endif   
    return failure;
  }
  char *data = (char*)malloc(plSz);
  assert(data);
  for (uint8_t i = 0; i < plSz; i++) {
    data[i] = ctx->adu.pdu.data[ replyData + i ];
  }
  mbr->lastValid = (uint16_t)strtol(data, NULL, 10);
#ifndef NDEBUG
  printf("Info: New value %d \n", mbr->lastValid);
#endif  
  return done;
};

/**
 * @brief  Initialize the MBAP of ADU
**/
int mbInitMbap(mbCtx *ctx) {
  assert(ctx);
  uint16_t tID, pID, fBytes;
  uint8_t uID;
  tID    = (uint16_t)strtol(mbrValue(ctx->dev.currMapMbr, functionCode) , NULL, 10);
  pID    = (uint16_t)strtol(mbrValue(ctx->dev.currMapMbr, protocol) , NULL, 10);
  fBytes = (uint16_t)strtol(mbrValue(ctx->dev.currMapMbr, mbr) , NULL, 10);
  uID = (uint8_t)ctx->adu.mbap.uID;
  ctx->dev.txVector[ tIDLsb ] = (uint8_t)LTOS_LSB(tID); /* BEGIN QUERY FRAME */
  ctx->dev.txVector[ tIDMsb ] = (uint8_t)LTOS_MSB(tID);
  ctx->dev.txVector[ pIDLsb ] = (uint8_t)LTOS_LSB(pID);
  ctx->dev.txVector[ pIDMsb ] = (uint8_t)LTOS_MSB(pID);
  ctx->dev.txVector[ dSZLsb ] = (uint8_t)LTOS_LSB(uID);
  ctx->dev.txVector[ dSZMsb ] = (uint8_t)LTOS_MSB(uID);
  ctx->dev.txVector[ uID    ] = uID;

  (uint16_t)ctx->adu.mbap.tID;
(uint16_t)ctx->adu.mbap.pID;
(uint16_t)ctx->adu.mbap.fBytes;

  return (_mbap_size_);
};

/**
 * @brief  Initialize the PDU of ADU
**/
int mbInitPdu(mbCtx *ctx) {
  assert(ctx && ctx->dev.currMapMbr);
  _mbrNode *mbr = ctx->dev.currMapMbr;
  uint8_t fCode = (uint8_t)strtol(mbrValue(mbr, functionCode) , NULL, 10);
  uint16_t mbrAddress = (uint16_t)strtol(mbrValue(mbr, mbrAddress) , NULL, 10);
  uint16_t mbrSize = (uint16_t)strtol(mbrValue(mbr,   size) , NULL, 10);
  ctx->dev.txVector[functionCode] = fCode;
  ctx->dev.txVector[registerAddrLsb] = (uint8_t)LTOS_LSB(mbrAddress);
  ctx->dev.txVector[registerAddrMsb] = (uint8_t)LTOS_MSB(mbrAddress);
  ctx->dev.txVector[registerSizeLsb] = (uint8_t)LTOS_LSB(mbrSize);
  ctx->dev.txVector[registerSizeMsb] = (uint8_t)LTOS_MSB(mbrSize);  /* END QUERY FRAME */
  ctx->adu.pdu.functionCode = fCode;
  ctx->adu.pdu.mbrAddress = mbrAddress;
  ctx->adu.pdu.mbrSize = mbrSize;
  return (done);
}; 

/**
 * @brief  Send request to a modbus device
**/
int mbSendRequest(mbCtx *ctx, _mbrNode *mbr){
  assert(ctx && ctx->dev.link.modbusTcp.socket && ctx->dev.currMapMbr);
  mbInitMbap(ctx); /* copy data from internal structure to txVector */
  mbInitPdu(ctx);
#ifndef NDEBUG
  _mbRequestRaw(ctx);
#endif  
  if (send(ctx->dev.link.modbusTcp.socket, ctx->adu.pdu.data, _adu_size_, MSG_DONTWAIT) != _adu_size_) {
    puts("Error: Can't send data to device.");
    return failure;
  }
  return done;
};

/**
 * @brief Wait for reply from a modbus device for a maximum specified delta time 
**/
uint32_t waitReply(mbCtx *ctx){ /*//TODO Implement some tunning for wait */
  uint32_t receiveTimeout = (uint32_t)( ctx->dev.link.modbusTcp.msTimeout * 10E2 );
  struct timeval wait;
  wait.tv_sec = 0; 
  wait.tv_usec = receiveTimeout;
  fd_set fd;
  FD_ZERO(&fd);
  FD_SET(ctx->dev.link.modbusTcp.socket, &fd);
  if( select(FD_SETSIZE, &fd, NULL, NULL, &wait) == failure){ /* wait for data wait.tv_usec */
    return failure;
  }
  return(receiveTimeout - wait.tv_usec);
};

/**
 * @brief  After a request is sended to slave this function get and process the reply
**/
int mbGetReply(mbCtx *ctx, _mbrNode *mbr) {
  assert(ctx && ctx->dev.link.modbusTcp.socket);
  uint32_t replyDelay = waitReply(ctx);
  if(replyDelay == failure){
#ifndef NDEBUG
    puts("Error: Device reply timeout");
#endif
    return failure;    
  }
  uint8_t replySize = recv(ctx->dev.link.modbusTcp.socket, ctx->adu.pdu.data, _adu_size_, MSG_DONTWAIT);
#ifndef NDEBUG
  _mbReplyRaw(ctx);
  printf("Info: Reply time    : ~%.02f ms\n", replyDelay/10E2);
#endif 
  if( replySize < (reply_exception) ) { /* Unknown modbus reply */
#ifndef NDEBUG
    puts("Error: Unknown modbus reply");
    puts("Error: To short reply");
#endif
    return failure;
  }
  if( replySize == (reply_exception) ) { /* Exception received */
#ifndef NDEBUG
    puts("Error: Exception received");  
#endif
    return failure;
  }
  if( replySize == (reply_size_max) ) { /* Requested data received?!?! */
    return mbParseReply(ctx, mbr);
  }
#ifndef NDEBUG
  puts("Error: Unknown modbus reply");
  puts("Error: Are you connected to a modbus device?");
#endif 
  return failure;
};

/**
 * @brief  Update context device modbus registers (MBR)
**/
int mbUpdate(mbCtx *ctx) {
  assert(ctx && ctx->dev.headMap);
  uint8_t commFailure = 0;
  _mbrNode *current = ctx->dev.headMap;
  while(current){ 
    ctx->dev.currMapMbr = current;
    if ( mbSendRequest(ctx, current) == failure || mbGetReply(ctx, current) == failure ) { /* Dont call mbGetAnswer if mbSendRequest fails */
      mbTcpReconnect(ctx);
    }
    current = current->next;
  }

  return( commFailure?( failure ):( done ) );
};