/**
 * @file   mbTcp.h
 * @author Marcelo Niendicker Grando
 * @date   2021/09/13
 *
 * @brief  
 *        Do the transactions = queries to modbus slave device using Modbus TCP protocol
 *
 * @see Modbus specification
 * @see https://modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
 * @see https://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf
 * @see https://www.ni.com/pt-br/innovations/white-papers/14/the-modbus-protocol-in-depth.html
 */

#ifndef MB_TCP
#define MB_TCP

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>     /* malloc() */
#include <unistd.h>     /* read() and write() */
#include <string.h>	    /* strlen */
#include <sys/time.h>   /* struct timeval... */
#include <sys/socket.h>                            
#include <arpa/inet.h>	/* inet_addr */
#include <netdb.h>	    /* hostent */
#include "mbDevice.h"

enum mbFunctionCode {       /* Just supported                  */
  readCoils = 1,            /* Device digital inputs           */
  readDiscreteInputs = 2,   /* Device analogue inputs          */
  readHoldingRegisters = 3, /* Device configuration parameters */
  readInputRegisters = 4    /* Device measurement variables    */
};


enum mbExceptionCode{
  illegalFunction=1,        /* 01 (0x01)  | The function code received in the request
                                          |   is not an authorized action for the slave. 
                                          |   The slave may be in the wrong state to 
                                          |   process a specific request.                   */
  illegalDataAddress,       /* 02 (0x02)  | The data address received by the slave 
                                          |   is not an authorized address for the slave.   */
  illegalDataValue,         /* 03 (0x03)  | The value in the request data field        
                                          |   is not an authorized value for the slave.     */
  slaveSeviceFailure,       /* 04 (0x04)  | The slave fails to perform a requested action 
                                          |   because of an unrecoverable error.            */
  acknowledge,              /* 05 (0x05)  | The slave accepts the request but needs
                                          |   a long time to process it. */
  slaveDeviceBusy,          /* 06 (0x06)  | The slave is busy processing another command. 
                                          |   The master must send the request once
                                          |   the slave is available.                      */
  negativeAcknowledgment,   /* 07 (0x07)  | The slave cannot perform the programming 
                                          |   request sent by the master.                  */
  memoryParityError,        /* 08 (0x08)  | The slave detects a parity error in the memory 
                                          |   when attempting to read extended memory.     */
  gatewayPathUnavailable,   /* 10 (0x0A)  | The gateway is overloaded or not correctly 
                                          |   configured.                                  */
  gatewayNotFound           /* 11 (0x0B)  | The slave is not present on the network.       */
};

enum MBAPi { /* MBAP Data Index from ADU[0] for [tx/rxVector] */
  tIDMsb = 0, tIDLsb, /* 16 bits value */
  pIDMsb, pIDLsb, 
  dSZMsb, dSZLsb, 
  uID, /* 8 bits value */
  _mbap_size_
};

enum requestPDUi { /* PDU data index from ADU[sizeof(MBAP)]*/
  functionCode = (_mbap_size_),
  registerAddrMsb,
  registerAddrLsb,
  registerSizeMsb,
  registerSizeLsb,
  _adu_size_ /* Reference for modbus QUERY/REPLY MAXIMUM SIZE */
};           /* MBAP + PDU = Modbus message */

enum replyPDUi { /* Reply PDU data index */
  replyFC = (_mbap_size_),
  replySz, /* payload bytes */
  replyData /* payload start data*/
};

enum _aduBytes { /* From Modbus specification */
  adu_size_max = 260,      /* Absolute Maximum bytes in Request/Reply ADU */
  mbap_size = (_mbap_size_),
  fc_size = 1,
  payload_size_max = ( (adu_size_max) - (mbap_size) - (functionCode) ),
  reply_exception = 2,
  reply_size_min = ( (_mbap_size_) + (reply_exception) ),
  reply_size_max = (_adu_size_)
};

typedef struct { /* MODBUS APPLICATION PROTOCOL HEADER - MBAP  */
  __uint16_t tID; /* IDentification of a MODBUS Request/Response transaction */
  __uint16_t pID; /* 0 = MODBUS protocol */
  __uint16_t fBytes; /* Number of following bytes(unitID + FC + FC specific data) */
  __uint8_t uID; /* Device address connected on a serial line or on other buses */
} _aduMBAP;

typedef struct { /* PROTOCOL DATA UNIT - PDU */
  uint8_t functionCode; /* Define a modbus operation. ex: 0x04 = Read Input Register  */
  uint32_t mbrAddress;
  uint32_t mbrSize;
  //uint8_t data[_adu_size_]; /* Contains additional information that the server uses */
                            /* to take the action defined by the function code */
                            /* This vector is used as txVector and rxVector */
                            /* Function code dependant */
} _aduPDU;

typedef struct { /* MODBUS TCP APPLICATION DATA UNIT - ADU */
  _aduMBAP mbap;
  _aduPDU pdu;
} _tcpADU;

typedef struct { /* Modbus Device context */
  _tcpADU adu;
  device dev;
} mbCtx;

/**
 * @brief Load device modbus parameters from @filePath including registers map filepath
 *        Load device modbus registers(MBR) map
 *        Initialize all context data
 * @param filePath Path to device configuration file
 * @return bmCtx|NULL
 */
mbCtx *mbInit(const char *mbDevConfigFile);

/**
 * @brief Open and set context socket, Resolve device Hostname and Connect to IP address
 *        If a connection could be established save IP address(for running instance) for next connections
 * @param ctx Context with meta data to stablish connection
 * @return done|failure
 */
int mbTcpConnect(mbCtx *ctx);

/**
 * @brief  Update context device modbus registers (MBR)
 * @param  ctx Context with register map to update.
 * @return done|failure 
 */
int mbUpdate(mbCtx *ctx);

/**
 * @brief Close context link socket and set it to -1
 * @param ctx Context with meta data to socket.
 * @return done|failure
 */
int mbTcpDisconnect(mbCtx *ctx);

/**
 * @brief Free dynamic allocated memory 
 * @param ctx Device Context
 * @return done|failure
 */
int mbClose(mbCtx *ctx);

/**
 * @brief Close and reopen socket connection 
 *
 * @param ctx Context with meta data to socket.
 * @return done|failure
 */
int mbTcpReconnect(mbCtx *ctx);

/**
 * @brief Create a new context.
 *        Load all device parameters from filePath file
 * @param filePath Path to device configuration file
 * @return bmCtx|NULL
 */
mbCtx * mbLoadConf(const char * filePath);

/**
 * @brief Print device configuration
 * @param ctx device context
 * @return done|failure
 */
int mbShowConf(const mbCtx *ctx);

/**
 * @brief Load all device's registers map metadata
 * @param ctx With device's modbus registers metadata filepath
 * @return ctx|NULL
 */
mbCtx *mbLoadMap(mbCtx *ctx);

/**
 * @brief  Print device registers map information
**/
int mbShowRegistersMap(const mbCtx *ctx);

/**
 * @brief Receive a hostname and try to get and return the IP address
 * @param hostname Network name of the slave device
 * @return ipAddress|NULL.
 */
char *htoip(char *hostname);

/**
* @brief Initialize the MBAP of ADU
* @param ctx Context with MBAP data.
* @param txVector The vector used for transmit the modbus query
* @return next index on txVector | failure
*/
int mbInitMbap(mbCtx *ctx);

/**
 * @brief  Initialize the PDU of ADU
 * @param  ctx Context with current register with data for PDU.
 * @return done
 */
int mbInitPdu(mbCtx *ctx);

/**
 * @brief  Send request to a modbus device
 * @param  ctx Modbus device context
 * @param  mbr Modbus register 
 * @return done|failure
 */
int mbSendRequest(mbCtx *ctx, _mbrNode *mbr);

/**
 * @brief Print to stdout sendded ADU raw data
 *
 * @param ctx Modbus device context.
 * @return done|failure
 */
int _mbRequestRaw(const mbCtx *ctx);

/**
 * @brief Wait for reply from a modbus device for a maximum specified delta time 
 * @param ctx Modbus device Context with socket and timeout 
 * @return delta time in uS|failure
 */
uint32_t waitReply(mbCtx *ctx);

/**
 * @brief  After a request is sended to slave this function get and process the reply
 *
 * @param  ctx Modbus device Context
 * @param  mbr Modbus register with data for PDU
 * @return done|failure
 */
int mbGetReply(mbCtx *ctx, _mbrNode *mbr);

/**
 * @brief Print to stdout received ADU raw data
 *
 * @param ctx Modbus device context.
 * @return done|failure
 */
int _mbReplyRaw(const mbCtx *ctx);

/**
 * @brief When a correct modbus reply data size is received
 *        this function parse and save received data to ctx  
 *
 * @param  ctx Modbus device Context
 * @param  mbr Modbus register with data for check
 * @return done|failure
 */
int mbParseReply(mbCtx *ctx, _mbrNode *mbr);

#endif /* ./include/modbusTcp.h */