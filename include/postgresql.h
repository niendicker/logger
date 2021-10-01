#ifndef POSTGRESQL
#define POSTGRESQL

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

struct dataFileContext {
  char quote;
  char delimiter;
  char *absoluteFilePath;
  char *format;
  char *header;
};


struct sqlContext{
  char *auth; 
  char *user;
  char *database;
  char *schema;
  char *table; /* Must be full path [ database.schema.table ] */
  char *sqlFile;
};

/**
 * @brief  Generate timestamp with time zone. 'YYYY-MM-DD HH:MM:SS~TZ'
**/
char *timestampz();

/**
 * @brief  Execute the sqlFile against postgres using psql interface
**/
int runSql(char *sqlFile, ...);

int storePing(char *devID, int ping_ms);



#endif /* postgresql.h */