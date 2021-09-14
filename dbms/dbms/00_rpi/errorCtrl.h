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
#ifndef ERROR_CTRL
#define ERROR_CTRL

#include <assert.h>
#include <errno.h>
extern int errno;
#define printError()  printf("From file: %s Line: %d\n", __FILE__, __LINE__ ); \
                      strerror(errno);

#endif