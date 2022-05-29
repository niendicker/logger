/**
 * @file   error.h
 * @author Marcelo Niendicker Grando
 * @date   2021/09/13
 * @brief  Basics status definitions
*/ 

#ifndef ERROR_CTRL
#define ERROR_CTRL
//#define NDEBUG

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

typedef enum stdFunctionReturn{ 
  failure=-1, 
  done=0
} _std_fr;

#define f_NULL(f) ( (f) == NULL )
#define f_DONE(f) ( (f) == (done) ) 
#define f_FAIL(f) ( (f) == (failure) )

#endif /* ./include/errorCtrl.h */