/**
 * @file   mbTcp.c
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
#define STOL(S_MSB, S_LSB) ( (int16_t)(S_MSB << 8) + S_LSB )

/**
 * @brief Load context config parameters from filePath file
 *        Load context config registers map (MBR) metadata
 *        Initialize context
**/
mbCtx *mbInit(const char *mbDevConfigFile) {
  assert(mbDevConfigFile);
  mbCtx *newDev = mbConfigure(mbDevConfigFile);
  if ( !newDev ) { return NULL; }
  newDev->adu.mbap._tID = 0;
  newDev->adu.mbap._pID = newDev->dev.link.protocol;
  newDev->adu.mbap._uID = newDev->dev.link.modbusRtu.unitAddress; /* Serial line address */
  newDev->adu.mbap._fBytes = 0;
  newDev->dev.link.modbusTcp.socket = failure;
  newDev->dev.txADU = salloc(_adu_size_);
  newDev->dev.rxADU = salloc(_adu_size_);
#ifndef QUIET_OUTPUT
  mbShowConf(newDev);    
  mbShowRegistersMap(newDev);
#endif    
  return newDev;
};

/**
 * @brief Open a socket, Resolve config Hostname and Connect to IP address
 * @param ctx Context with meta data to stablish a connection
 * @return done|failure
 */
int mbTcpConnect(mbCtx *ctx) {
  assert(ctx);
  int *fdSocket = &ctx->dev.link.modbusTcp.socket; 
  *fdSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  assert(fdSocket);
  char *ipAddress = ctx->dev.link.modbusTcp.ipAddress;
  uint64_t ipHash = djb2_hash(ipAddress);
  char *defaultIP = (char*)"000.000.000.000";
  uint64_t defaultIpHash = djb2_hash(defaultIP);
  if(ipHash == defaultIpHash){ /* Invalid IP. Trying DNS translation from hostname */
    char *hostname = salloc_init(confValue(ctx->dev.config, hostname));
    assert(hostname);
    ipAddress = srealloc_copy(ipAddress, htoip(hostname)); /* overwrite IP parameter for current instance */
  }
  struct sockaddr_in mbServer; /* Set connection parameters */
  mbServer.sin_addr.s_addr = inet_addr(ipAddress);
  mbServer.sin_family = AF_INET;  
  mbServer.sin_port = htons(ctx->dev.link.modbusTcp.port);
#ifndef QUIET_OUTPUT
  printf("Info: Connecting to %s @%s:%d \n", confValue(ctx->dev.config, tag), ipAddress, ctx->dev.link.modbusTcp.port);
#endif
  struct timeval timeout;
  timeout.tv_sec  = 7;  // after 7 seconds connect() will timeout
  timeout.tv_usec = 0;
  setsockopt(*fdSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  if (connect(*fdSocket, (struct sockaddr *)&mbServer, sizeof(mbServer)) == failure) {
#ifndef QUIET_OUTPUT
    printf("Error: Connection refused from %s \n", confValue(ctx->dev.config, tag));
#endif
    return failure;
  } 

#ifndef QUIET_OUTPUT
  printf("Info: Connected to %s \n", confValue(ctx->dev.config, tag));
#endif
  return done;
};

/**
 * @brief  Update context config modbus registers (MBR)
**/
int mbUpdate(mbCtx *ctx) {
  assert(ctx && ctx->dev.mbr);
  uint8_t commFailure = 0;
  _ln *mbr = ctx->dev.mbr;
  while(mbr){ 
    if ( mbSendRequest(ctx, mbr) == failure || mbGetReply(ctx, mbr) == failure ) { /* Dont call mbGetReply if mbSendRequest fails */
      mbTcpReconnect(ctx);
      commFailure++;
    }
    mbr = mbr->next;
  }
  return( commFailure?( failure ):( done ) );
}; /* mbUpdate */

/**
 * @brief Close socket connection
 */
int mbTcpDisconnect(mbCtx * ctx) {
  assert(ctx);
  int32_t *socket = &ctx->dev.link.modbusTcp.socket;
  if(socket > 0)
    close(*socket);
  *socket = 0;
#ifndef QUIET_OUTPUT
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
  ctx->adu.mbap._tID = 0;
  ctx->adu.mbap._pID = 0; //Modbus protocol
  ctx->adu.mbap._uID = 0; //Serial line or subnet address
  ctx->adu.mbap._fBytes = 0;
  ctx->dev.link.modbusTcp.socket = failure;
  ctx->dev.link.modbusTcp.msTimeout = (uint8_t)min_timeout_ms; /* milli seconds */
  ctx->dev.link.modbusRtu.msTimeout = (uint8_t)min_timeout_ms;
  ctx = NULL;
  return done;
};

/**
 * @brief Close and reopen socket connection 
**/
int mbTcpReconnect(mbCtx *ctx){
  assert(ctx);
  mbTcpDisconnect(ctx);
  mbTcpConnect(ctx);
  return done;
};

/**
 * @brief Create a new context.
 *        Load all config parameters from filePath file
**/
mbCtx * mbConfigure(const char * filePath) {
  assert(filePath);
  mbCtx *ctx = (mbCtx *) calloc(sizeof(mbCtx), _byte_size_);
  assert(ctx);
  deviceConfigure(&ctx->dev, filePath);
  return ctx;
};

/**
 * @brief  Print config configuration
**/
int mbShowConf(mbCtx *ctx) {
   showDeviceConf(&ctx->dev);
   return done;
};

/**
 * @brief  Print config registers map information
**/
int mbShowRegistersMap(mbCtx *ctx) {
  showDeviceMap(&ctx->dev);
  return done;
};

/**
 * @brief Receive a hostname and try to get and return the IP address
 * @param hostname Network name of the slave config
 * @return ipAddress|NULL.
 */
char *htoip(char *hostname) {
  struct hostent *he = gethostbyname(hostname);
  assert(he);
  struct in_addr **addr_list = (struct in_addr**)he->h_addr_list; /* Cast since h_addr_list also has the ip address in long format only */
  assert(*addr_list);
  char *ip = salloc_init(inet_ntoa(*addr_list[0]));
#ifndef QUIET_OUTPUT
  printf("Info: DNS translation finished for %s to IP %s \n", hostname, ip);
#endif
  return (ip != NULL ? ip : NULL);
};

/**
 * @brief  Initialize the MBAP of ADU for current transaction
**/
int mbInitMBAP(mbCtx *ctx) {
  assert(ctx);
  uint16_t tID, pID, fBytes;
  uint8_t uID;
  tID    = (uint16_t)rand();
  pID    = ctx->dev.link.protocol;
  fBytes = _adu_size_ - _uID;
  uID    = ctx->dev.link.modbusRtu.unitAddress;
  ctx->dev.txADU[ _tIDLsb ] = (uint8_t)LTOS_LSB(tID); /* BEGIN QUERY FRAME */
  ctx->dev.txADU[ _tIDMsb ] = (uint8_t)LTOS_MSB(tID);
  ctx->dev.txADU[ _pIDLsb ] = (uint8_t)LTOS_LSB(pID);
  ctx->dev.txADU[ _pIDMsb ] = (uint8_t)LTOS_MSB(pID);
  ctx->dev.txADU[ _dSZLsb ] = (uint8_t)LTOS_LSB(fBytes);
  ctx->dev.txADU[ _dSZMsb ] = (uint8_t)LTOS_MSB(fBytes);
  ctx->dev.txADU[ _uID    ] = uID;
  ctx->adu.mbap._tID    = tID;
  ctx->adu.mbap._pID    = pID;
  ctx->adu.mbap._fBytes = fBytes;
  ctx->adu.mbap._uID    = uID;
  return done;
};

/**
 * @brief  Initialize the PDU of ADU
**/
int mbInitPDU(mbCtx *ctx, uint8_t fCode, uint16_t mbrAddress, uint16_t mbrSize) {
  assert(ctx && ctx->dev.mbr);
  //_ln *mbr = ctx->dev.mbr;
  //uint8_t fCode = (uint8_t)strtol(mbrValue(mbr, function) , NULL, 10);
  //uint16_t mbrAddress = (uint16_t)strtol(mbrValue(mbr, address) , NULL, 10);
  //uint16_t mbrSize = (uint16_t)strtol(mbrValue(mbr,   size) , NULL, 10);
  ctx->dev.txADU[ _fCode      ] = fCode;
  ctx->dev.txADU[ _mbrAddrLsb ] = (uint8_t)LTOS_LSB(mbrAddress);
  ctx->dev.txADU[ _mbrAddrMsb ] = (uint8_t)LTOS_MSB(mbrAddress);
  ctx->dev.txADU[ _mbrSizeLsb ] = (uint8_t)LTOS_LSB(mbrSize);
  ctx->dev.txADU[ _mbrSizeMsb ] = (uint8_t)LTOS_MSB(mbrSize);  /* END QUERY FRAME */
  ctx->adu.pdu.functionCode = fCode;
  ctx->adu.pdu.mbrAddress = mbrAddress;
  ctx->adu.pdu.mbrSize = mbrSize;
  return (done);
}; 

/**
 * @brief  Send modbus query to device
**/
int mbSendRequest(mbCtx *ctx, _ln *mbr){
  assert(ctx && ctx->dev.link.modbusTcp.socket && ctx->dev.mbr);
  mbInitMBAP(ctx); /* copy data from internal structure to txVector */
  uint8_t fCode = (uint8_t)strtol(mbrValue(mbr, function) , NULL, 10);
  uint16_t mbrAddress = (uint16_t)strtol(mbrValue(mbr, address) , NULL, 10);
  uint16_t mbrSize = (uint16_t)strtol(mbrValue(mbr,   size) , NULL, 10);
  mbInitPDU(ctx, fCode, mbrAddress, mbrSize);
#ifndef QUIET_OUTPUT
  _mbRequestRaw(ctx);
#endif 
  if (send(ctx->dev.link.modbusTcp.socket, ctx->dev.txADU, _adu_size_, MSG_DONTWAIT) != _adu_size_) {
#ifndef QUIET_OUTPUT
    puts("Error: Can't send data to config.");
#endif
    return failure;
  }
  return done;
};

/**
 * @brief Print to stdout sendded ADU raw data
**/
int _mbRequestRaw(const mbCtx *ctx){
  assert(ctx);
  printf("Info: Modbus Query: ");
  for (int i = 0; i < _adu_size_; i++){
    uint8_t data = (uint8_t)ctx->dev.txADU[i];
    printf("%02X", data); /* Hex value */
  }
  printf("\n");
  return done;
};

/**
 * @brief Wait for a reply from modbus device, until a maximum elapsed time (receiveTimeout[ms]) 
**/
uint32_t waitReply(mbCtx *ctx){ /*//TODO Implement some tunning for wait */
  uint32_t receiveTimeout = (uint32_t)( ctx->dev.link.modbusTcp.msTimeout * 10E2 );
  struct timeval wait;
  wait.tv_sec = 0; 
  wait.tv_usec = receiveTimeout; /* timeout for select(...) */
  fd_set fd;
  FD_ZERO(&fd); /* Reset file descriptor */
  FD_SET(ctx->dev.link.modbusTcp.socket, &fd);
  /* wait for data ready on file descriptor */
  if( select(FD_SETSIZE, &fd, NULL, NULL, &wait) == failure){ 
    return failure;
  }
  return(receiveTimeout - wait.tv_usec);
};

/**
 * @brief Print to stdout received ADU raw data
**/
int _mbReplyRaw(const mbCtx *ctx){
  assert(ctx);
  printf("Info: Modbus Reply: ");
  for (size_t i = 0; i < _adu_size_; i++){
    uint8_t data = (uint8_t)ctx->dev.rxADU[i];
    printf("%02X", data); /* Hex value */
  }
  printf("\n");
  return done;
};

/**
 * @brief Interpret and update lstValid for specific mbr
 */
int mbUpdateValue(mbCtx *ctx, _ln *mbr){
  uint8_t plSz = (uint8_t)ctx->dev.rxADU[_replySZ]; /* Reply payload size */ 
  char *payload = (char*)calloc(plSz, _byte_size_);
  for (uint8_t i = 0; i < plSz; i++) {
    payload[i] = ctx->dev.rxADU[ _replyData + i ];
  }
  uint8_t fCode    = strtol(mbrValue(mbr, function), NULL, 10);
  uint8_t isSigned = strtol(mbrValue(mbr, signal  ), NULL, 10);
  int32_t raw_value = (int8_t)payload[0];
  float value = 0.0;
  if( fCode >= readHoldingRegisters ){ /* WORD SIZE(16bits) VALUES */
    int8_t msb = (int8_t)payload[0], lsb = (int8_t)payload[1];
    raw_value = STOL(msb, lsb);
  }
  else{ /* 8bits value */
    raw_value = payload[0];
  }
  int16_t scl = strtol(mbrValue(mbr, scale   ), NULL, 10);
  if(isSigned) 
    value = (float4_t)raw_value/scale;
  else 
    value = (float4_t)(((uint16_t)raw_value)/scl);
  char *floatValue = salloc(strlen("10000.00"));
  sprintf(floatValue, "%5.02f", value);
  updateValue(mbr, (char*)"lastValid", floatValue);      
  free(payload);
  free(floatValue);
  return done;
};

/**
 * @brief When a correct modbus reply data size is received
 *        this function parse and save received data to ctx  
**/
int mbParseReply(mbCtx *ctx, uint8_t replySize){
  assert(ctx);
  if(replySize < reply_size_min)
    return failure;
  uint16_t tID, pID, fBytes;
  uint8_t uID, fCD, plSz;
  tID  = (uint16_t)STOL(ctx->dev.rxADU[ _tIDMsb ], ctx->dev.rxADU[ _tIDLsb ]); /* MBAP */
  pID  = (uint16_t)STOL(ctx->dev.rxADU[ _pIDMsb ], ctx->dev.rxADU[ _pIDLsb ]); 
  fBytes = (uint16_t)STOL(ctx->dev.rxADU[ _dSZMsb ], ctx->dev.rxADU[ _dSZMsb ]);
  uID  = (uint8_t)ctx->dev.rxADU[_uID]; 
  fCD  = (uint8_t)ctx->dev.rxADU[_replyFC]; /* PDU */ 
  plSz = (uint8_t)ctx->dev.rxADU[_replySZ]; /* Reply payload size */ 
  if(tID != ctx->adu.mbap._tID){  /* Transaction ID need to be the same for query/reply */
#ifndef QUIET_OUTPUT
    printf("Error: Transaction ID %d \n", tID);
#endif    
    return failure;
  }
  if(pID != ctx->adu.mbap._pID){     /* Standard 0 = modbus tcp  */
#ifndef QUIET_OUTPUT
    printf("Error: pID %d \n", pID);
#endif    
    return failure;
  }
  if(uID != ctx->dev.link.modbusRtu.unitAddress){ /* Unit ID is the modbus RTU (Serial comm.) address */
#ifndef QUIET_OUTPUT
    printf("Error: Unit ID %d \n", uID);
#endif    
    return failure;
  }
  uint8_t fcdRequest = ctx->adu.pdu.functionCode; 
  if(fCD == (fcdRequest + 0x80)){
#ifndef QUIET_OUTPUT
    printf("Error: Exception received %d \n", fCD);
#endif   
    return failure;
  }
  if(fBytes < 3 ){ /* At least 1B(uID) + 1B(fCode) + 1B(exception code) */
#ifndef QUIET_OUTPUT
    printf("Error: Data size to short %d \n", fBytes);
#endif    
    return failure;
  }
  if( (plSz < 1)||(plSz > _adu_size_)){
#ifndef QUIET_OUTPUT
    printf("Error: Payload size %d \n", plSz);
#endif   
    return failure;
  }
  return mbUpdateValue(ctx, ctx->dev.mbr);
};

/**
 * @brief  After a request is sended to slave this function get and process the reply
**/
int mbGetReply(mbCtx *ctx, _ln *mbr) {
  assert(ctx && ctx->dev.link.modbusTcp.socket);
  int replyDelay = waitReply(ctx);
  if(replyDelay == failure){
#ifndef QUIET_OUTPUT
    puts("Error: Device reply timeout");
#endif
    return failure;    
  }
  uint8_t replySize = recv(ctx->dev.link.modbusTcp.socket, ctx->dev.rxADU, _adu_size_, MSG_DONTWAIT);
#ifndef QUIET_OUTPUT
  _mbReplyRaw(ctx);
  printf("Info: Reply time  : ~%.02f ms\n", replyDelay/10E2);
#endif 
  return mbParseReply(ctx, replySize);
};

/**
 * @brief Save all modbus registers value 
**/
_ln* pushDeviceData(char *deviceID, _ln *deviceMbr){
  assert(deviceID && deviceMbr);
  _ln *deviceData = deviceMbr;
  _ln *newRow = NULL;
  char *colID = salloc(_byte_size_); 
  char *lastValidData = salloc(_byte_size_);
  while(deviceData){ 
    newRow = pushNode(newRow);
    /* FILO list */
    colID = srealloc_copy(colID, mbrValue(deviceData, meaning));
    char *dataType = (char*)"float4"; 
    newRow = pushData(newRow, colID, dataType);
    /* Insert data at head of list */
    lastValidData =  srealloc_copy(lastValidData, mbrValue(deviceData, lastValid)); /* last successfully readded data */
    newRow = pushData(newRow, colID, lastValidData);
    deviceData = deviceData->next;
  }
  free(colID);
  free(lastValidData);
  assert(newRow);
  return newRow;
}; /* pushDeviceData */

/**
 * @brief Free device data/type list
**/
int dropDeviceData(_ln *deviceRow){
  assert(deviceRow);
  char *columnID = salloc(_str_null_);
  while(deviceRow){ 
    columnID = srealloc_copy(columnID, deviceRow->data->key);
    _dn *data = deviceRow->data;
    while(data){
      deleteData(deviceRow, columnID);
      data = data->next;
    }
    deviceRow = deviceRow->next;
  }
  free(columnID);
  return 0;
}; /* dropDeviceData */

/**
 * @brief  Store the data on postgresql
**/
int saveData(mbCtx *_mbCtx){
  assert(_mbCtx);
  char *deviceID = confValue(_mbCtx->dev.config, tag);
  assert(deviceID);
  _ln *mbr = _mbCtx->dev.mbr;
  assert(mbr);
  _ln *deviceData = pushDeviceData(deviceID, mbr);
  for(int i=0; i < 5; i++){ /*  */
    if(persistData(deviceData, _mbCtx->dev.config) != NULL){
      dropDeviceData(deviceData);
      return 0;
    }
  }
  dropDeviceData(deviceData);
  return -1;
}; /* save data */
