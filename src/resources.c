#include "resources.h"

/**
 * @brief  djb2 hash function by Dan Bernstein 
**/
__uint64_t djb2_hash(const char *str) {
  __uint64_t hash = 5381;
  int c;
  for(; (c = (uint8_t)*str++); )
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
};

/**
 * @brief  Return the number of ASCII bytes of a integer 
**/
int str_digits(int n) { 
  int i, count; 
  for (i = 10, count = 0; ; i *= 10, count++) { 
    if ((int)n / i == 0) break;
  } 
  return count + 1; 
};

/**
 * @brief  Allocate new vector for string
**/
char *salloc(int size){ /* release version disable all asserts */
  int stringSize = size + _str_null_;
  char *str = (char*)calloc( stringSize, _byte_size_);
  assert(str);
  return (str != NULL ? str : NULL);
};

/**
 * @brief  Allocate/Initialize new vector for string
**/
char *salloc_init(char *strInit){ /* release version disable all asserts */
  assert(strInit);
  char *str = (char*)salloc( strlen(strInit) );
  assert(str);
  memcpy(str, strInit, strlen(strInit) + _str_null_);
  assert(str);
  return str;
};

/**
 * @brief  Reallocate the vector for string
**/
char *srealloc(char* pstr, int newSize){ /* release version disable all asserts */
  pstr = (char*)realloc(pstr, newSize + _str_null_);
  assert(pstr);
  return( pstr != NULL ? pstr : NULL );
};

/**
 * @brief  Reallocate pstr with str size and copy it
**/
char *srealloc_copy(char* pstr, char* str){ /* release version disable all asserts */
  pstr = srealloc(pstr, strlen(str));
  assert(pstr);
  memcpy(pstr, str, strlen(str) + _str_null_);
  assert(pstr);
  return( pstr != NULL ? pstr : NULL );
};

/**
 * @brief  Return elapsed cpu time between two calls
**/
double cpu_time(_timer command){
  static clock_t begin = 0, end = 0;
  switch (command){
  case _start_: 
    begin = end = clock();
    break;
  default: /* check */
    end = clock();
    break;
  }
  return (((double)(end-begin))/CLOCKS_PER_SEC);
}