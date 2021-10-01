
#include "postgresql.h"

#define cache_size ((uint)10)

/**
 * @brief  Generate timestamp with time zone. 'YYYY-MM-DD HH:MM:SS~TZ'
**/
char *timestampz(){
  time_t now = time(&now);
  assert(now);
  struct tm *ptm = localtime(&now);
  assert(ptm);
  int tsz_length = sizeof("YYYY-MM-DD HH:MM:SS~TZ"); /* TZ = time zone offset + or - XX Hours */
  char *tsz = (char*)malloc(tsz_length);
  assert(tsz);
  strftime(tsz, tsz_length, "%F %T-03", ptm );
  assert(tsz);
  return tsz;
};



/**
 * @brief  Execute the sqlFile against postgres using psql interface
**/
int runSql(char *sqlFile, ...){
  assert(sqlFile);
  assert(strlen(sqlFile));
#ifdef __arm__
  char pwd[]  = "PGPASSWORD='n13nd1ck3r'";
#else
  char pwd[] = "PGPASSFILE='/home/dev/dbms/00_rpi/bin/.pgpass'";
#endif
  char psql[] = "psql"; /* (-q)Be quiet, (-U)user=modbuspoll, (-f) run SQL file */
  char pgUser[] = "modbuspoll"; 
  char *cmd = (char*)malloc(sizeof(pwd)+sizeof(psql)+strlen(sqlFile));
  assert(cmd);
  sprintf(cmd, "%s %s -U %s -f %s", pwd, psql, pgUser, sqlFile);
  int s = system(cmd);
  free(cmd);
  return s;
};

int storePing(char *devID, int reply_delay_ms){
  assert(devID);
  static FILE *csv_file;
  static u_int32_t cache = 0;
  char csv_filename[] = "mb_reply_delay.csv";
  if(cache == 0){
    csv_file = fopen(csv_filename, "w+");
    assert(csv_file);
  }
  assert(csv_file);
  char *tsz = timestampz();
  assert(tsz);
  fprintf(csv_file, "'%s',%.02f,%s,'ms'\n", devID, reply_delay_ms/10E2, tsz);
  free(tsz);
  cache++;
  if( cache >= cache_size ){
    fclose(csv_file);
    chmod(csv_filename, S_IRWXU |	S_IRWXG |	S_IRWXO);
#ifdef __arm__ /* arm-none-eabi-gcc compiler definition */
    char sqlFile[] = "home/pi/run/bin/sql/import_mb_reply_delay.sql";
#else
    char sqlFile[] = "/home/dev/dbms/00_rpi/bin/sql/import_mb_reply_delay.sql";
#endif

    runSql(sqlFile);
    cache = 0;
  }



  return 0;
};