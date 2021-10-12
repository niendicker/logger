#ifndef POSTGRESQL
#define POSTGRESQL

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "linkedlist.h"
#include "resources.h"

/* Postgresql types */ /* not compliant */
typedef char* varchar_t;    
typedef char* timestampz_t;
typedef float  float4_t;
typedef double float8_t;
#define _pgsql_varchar_    ((char*)"varchar"    )
#define _pgsql_real_       ((char*)"real"       )
#define _pgsql_timestamp_ ((char*)"timestamptz" ) 
#define _timestampz_size   (sizeof("YYYY-MM-DD HH:MM:SS~TZ") )/* TZ = +/- XX Hours offset */
/* Postgres connection */
#define _pgsql_interface_ ((char*)"psql"     )
#define _pgsql_user_      ((char*)"postgres" )
#define _pgsql_database_  ((char*)"postgres" )
#define _pgsql_host_      ((char*)"localhost")
#define _pgsql_port_      ((uint) 5432 )
/* Modbuspoll connection */
#define _mbpoll_          ((char*)"modbuspoll")
#define _mbpoll_user_     ( _mbpoll_ )
#define _mbpoll_database_ ( _mbpoll_ )
#define _mbpoll_schema_   ( _mbpoll_ )
#define _mbpoll_table_    ( _mbpoll_ )
#ifdef __arm__ /* arm-none-eabi-gcc compiler definition */
  #define _mbpoll_dataDir_  ((char*)"/home/pi/run/bin/"           )
  #define _mbpoll_sqlDir_   ((char*)"/home/pi/run/bin/sql/"       )
#else /* Using default for modbuspoll project */
  #define _mbpoll_dataDir_  ((char*)"/home/dev/dbms/00_rpi/bin/" )
  #define _mbpoll_sqlDir_   ((char*)"/home/dev/dbms/00_rpi/bin/sql/" )
#endif
/* Modbuspoll data export  */
#define _sql_template_copy_    ((char*)"template_copy.sql")
#define _sql_template_line_    ((uint)200)
#define _csv_file_             ((char*)"mbpoll.csv")
#define _csv_timestamp_header_ ((char*)"local_timestamp,") /* CSV default column */
#define _csv_                  ((char*)"csv"  )
#define _csv_std_delimiter_    (',')
#define _csv_delimiter_size_   (sizeof(""))
#define _csv_quote_size_       (sizeof("'"))

enum __csv{ /* Comma Separeted File */
  _csv_squote    = '\'' ,
  _csv_dquote    = '"'  ,
  _csv_comma     = ','  ,
  _csv_semicolon = ';'  ,
  _csv_space     = ' '
};

typedef struct __csvContext { 
  double persist_dt;
  char *fileName;
  char *filePath; /* Absolute file path 
  Must be accessible (r--) by postgres backend */
} _csvCtx; /* [pid]_mbpoll.csv */

typedef struct __sqlContext {
  char *pid; /* Is used to generate unique cvs filename*/
  char *hostname;
  int16_t port;
  char *auth; 
  char *user;
  char *database;
  char *schema;
  char *table;
  char *sqlTemplate; /* Sql script template. tags: %s %d %f... */
  _csvCtx inoutFile;
} _sqlCtx;

_sqlCtx *sqlCtxInit(_sqlCtx *sqlCtx, _ln *deviceConfig, _ln *deviceData);

int sqlCtxFree(_sqlCtx *sqlCtx);

int sqlAddColumns(_sqlCtx *ctx,  _ln *deviceData);

/**
 * @brief  Create a new table if not exists.
**/
int sqlCreateTable(_sqlCtx *ctx);

/**
 * @brief  Generate timestamp with time zone. 'YYYY-MM-DD HH:MM:SS~TZ'
**/
char *timestampz();

_sqlCtx *persistData(_ln *data, _ln *deviceConfig);

/**
 * @brief  Execute the sqlFile against postgres using psql interface
**/
int sqlImportCsv(_sqlCtx *sqlCtx);

char *insertCsvHeader(_ln *deviceData);

char *appendCsvData(_ln *deviceData, char *row);

int runSql(_sqlCtx *ctx, char *query);

#endif /* postgresql.h */