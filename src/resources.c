#include "resources.h"

__uint64_t djb2_hash(const char *str) {
  __uint64_t hash = 5381;
  int c;
  for(; (c = (uint8_t)*str++); )
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
};

int str_digits(int n) { 
  int i, count; 
  for (i = 10, count = 0; ; i *= 10, count++) { 
    if ((int)n / i == 0) break;
  } 
  return count + 1; 
};

char *salloc(int size){ /* release version disable all asserts */
  int stringSize = size + _str_null_;
  char *str = (char*)calloc( stringSize, _byte_size_);
  assert(str);
  return (str != NULL ? str : NULL);
};

char *salloc_init(char *strInit){ /* release version disable all asserts */
  assert(strInit);
  int stringSize = strlen(strInit) + _str_null_;
  char *str = (char*)salloc( stringSize );
  assert(str);
  memcpy(str, strInit, stringSize);
  assert(str);
  return str;
};

char *srealloc(char* pstr, int newSize){ /* release version disable all asserts */
  pstr = (char*)realloc(pstr, newSize + _str_null_);
  assert(pstr);
  return( pstr != NULL ? pstr : NULL );
};

char *srealloc_copy(char* pstr, char* str){ /* release version disable all asserts */
  pstr = srealloc(pstr, strlen(str));
  assert(pstr);
  memcpy(pstr, str, strlen(str) + _str_null_);
  assert(pstr);
  return( pstr != NULL ? pstr : NULL );
};
