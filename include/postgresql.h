#ifndef POSTGRESQL
#define POSTGRESQL

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>


#define pgsql_database "modbuspoll"
#define pgsql_relation "ping" 

int storePing(char *devID, int ping_ms);

#endif /* postgresql.h */