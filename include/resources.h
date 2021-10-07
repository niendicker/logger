/**
 * @file resources.h
 * @author Marcelo Niendicker Grando (marcelogrando@icloud.com)
 * @brief  Resource management functions
 * @version 0.1
 * @date 2021-10-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <time.h>  /* clock() */
#include <dirent.h> /* Directory stream functionalities */

#define _str_null_  ((uint8_t)1)
#define _byte_size_ ((uint8_t)1)


typedef enum __timer{ /* cpu_time() options */
  _start_,
  _check_
} _timer;

/**
 * @brief  djb2 hash function by Dan Bernstein 
 * 
 * @param str Vector to hash
 * @return hashed vector 
**/
__uint64_t djb2_hash(const char *str);

/**
 * @brief  Return the number of ASCII bytes of a integer 
 * 
 * @param n Integer to get number of digits when printed
**/
int str_digits(int n);

/**
 * @brief  Allocate new vector for string
 * 
 * @param size String size withow null terminator
**/
char *salloc(int size);

/**
 * @brief  Allocate/Initialize new vector for string
 * 
 * @param strInit Initialization data
**/
char *salloc_init(char *strInit);

/**
 * @brief  Reallocate the vector for string
 * 
 * @param pstr String to reallocate
 * @param newSize New string size
**/
char *srealloc(char* pstr, int newSize);

/**
 * @brief  Reallocate pstr with str size and copy it
 * 
 * @param pstr String to reallocate
 * @param str string to "fit" in pstr
**/
char *srealloc_copy(char* pstr, char* str);

/**
 * @brief  Return elapsed cpu time between two calls
 * 
 * @param command _start_ | _check_ 
 * @return elapsed cpu time 
**/
double cpu_time(_timer command);

char **getConfigs(char *configDir, char *fileExtension);


#endif /* resources.h */