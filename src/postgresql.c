
#include "postgresql.h"

#define _cache_time ((double)10/10E2) /* Elapsed time(s) between export data do postgres  */

_sqlCtx *sqlCtxInit(_sqlCtx *sqlCtx, _ln *deviceConfig, _ln *deviceData){
  assert(deviceConfig);
  sqlCtx = (_sqlCtx*)calloc(sizeof(_sqlCtx), _byte_size_);
  assert(sqlCtx);
  pid_t pid = getpid();
  sqlCtx->pid = salloc(str_digits(pid)); 
  sprintf(sqlCtx->pid, "%d", pid);
  assert(sqlCtx->pid);
  sqlCtx->hostname = salloc_init( peekValue(deviceConfig, (char*)"pgsqlHost") );
  sqlCtx->port     = salloc_init( peekValue(deviceConfig, (char*)"pgsqlPort") );
  char *auth       = salloc_init( peekValue(deviceConfig, (char*)"pgsqlAuth") );
  sqlCtx->auth     = salloc(strlen(auth));
  sprintf(sqlCtx->auth, auth, '='); /* put '=' signal  */
  sqlCtx->user     = salloc_init(peekValue( deviceConfig, (char*)"pgsqlUser")     );
  sqlCtx->database = salloc_init(peekValue( deviceConfig, (char*)"pgsqlDatabase") );
  sqlCtx->table    = salloc_init(peekValue( deviceConfig, (char*)"tag")           );
  /* File used to export data */
  sqlCtx->inoutFile.persist_dt = _cache_time;
  sqlCtx->inoutFile.fileName = salloc(strlen(sqlCtx->pid) + strlen(_csv_file_));
  sprintf(sqlCtx->inoutFile.fileName, "%s%s", sqlCtx->pid, _csv_file_); /* 12345678.csv */
  sqlCtx->inoutFile.filePath = salloc(strlen(_mbpoll_dataDir_) + strlen(sqlCtx->inoutFile.fileName));
  sprintf(sqlCtx->inoutFile.filePath, "%s%s", _mbpoll_dataDir_, sqlCtx->inoutFile.fileName); 
  free(auth);
  //static int TryOnce = true;/* Try once to create devices table */
  //if(TryOnce){
  //  TryOnce = false;
    if(sqlCreateTable(sqlCtx) == 0)
      sqlAddColumns(sqlCtx, deviceData);
  //}
  return sqlCtx;
};

int sqlCtxFree(_sqlCtx *sqlCtx){
  assert(sqlCtx);
  free(sqlCtx->table);
  free(sqlCtx->database);
  free(sqlCtx->user);
  free(sqlCtx->auth);
  free(sqlCtx->hostname); 
  free(sqlCtx->inoutFile.filePath);  
  free(sqlCtx);  
  return 0;
};

/**
 * @brief  Generate timestamp with time zone. 'YYYY-MM-DD HH:MM:SS~TZ'
**/
char *timestampz(){
  time_t now = time(&now);
  assert(now);
  struct tm *ptm = localtime(&now);
  assert(ptm);
  int tsz_length = _timestampz_size; 
  char *tsz = salloc(tsz_length);
  strftime(tsz, tsz_length, "%F %T-03", ptm );
  assert(tsz);
  return (tsz != NULL ? tsz : NULL);
};

int runSql(_sqlCtx *ctx, char *query){
  assert(ctx && query);
#ifndef QUIET_OUTPUT  
  char *psql = (char*)"psql";
#else 
  char *psql = (char*)"psql -q";  /* Be quiet */
#endif
  char *port = ctx->port;
  char cmdTemplate[] =  "%s %s --single-transaction --host=%s --port=%s --dbname=%s --username=%s --command=\"%s\"";
  uint cmdSize = strlen(ctx->auth) + strlen(psql)          + strlen(ctx->hostname) + 
                 strlen(port)      + strlen(ctx->database) + strlen(ctx->user) + 
                 strlen(query)     + sizeof(cmdTemplate);
  char *cmd = (char*)salloc(cmdSize);
  sprintf( /*Output  */ cmd, /*Template*/ cmdTemplate, 
           /*Context */ ctx->auth, psql, ctx->hostname, port, ctx->database, ctx->user, 
        /* sql query */ query);
  int s = system(cmd); /* run query */
  free(cmd);
  return s;
}

/**
 * @brief  Add column to devices table
**/
int sqlAddColumns(_sqlCtx *ctx, _ln *deviceData){
  assert(ctx && deviceData) ;
  char templateQuery[] = "ALTER TABLE modbuspoll.%s ADD if not exists %s %s NOT NULL;";
  char *query = salloc_init(templateQuery);
  char *columnID = salloc_init((char*)"local_timestamp" ); /*  Add timestamp*/
  char *dataType = salloc_init((char*)_pgsql_timestamp_);
  uint querySize = sizeof(templateQuery) + strlen(ctx->table) + strlen(columnID) + strlen(dataType);
  query = srealloc(query, querySize);
  sprintf(query, templateQuery, ctx->table, columnID, dataType);
  if(runSql(ctx, query) != 0 )
    return -1;
  _ln *data = deviceData;
  while(data){  /* Add others columns */
    columnID = srealloc_copy(columnID, data->data->key);
    dataType = srealloc_copy(dataType, _pgsql_real_);
    querySize = sizeof(templateQuery) + strlen(ctx->table) + strlen(columnID) + strlen(dataType);
    query = srealloc(query, querySize);
    sprintf(query, templateQuery, ctx->table, columnID, dataType);
    if(runSql(ctx, query) != 0 )
      break;
    data = data->next;  
  } 
  free(dataType);
  free(columnID);
  free(query);
  return 0;
};

/**
 * @brief  Create a new table if not exists.
**/
int sqlCreateTable(_sqlCtx *ctx){
  assert(ctx) ;
  char *tableID = ctx->table;
  char templateQuery[] = "create table if not exists modbuspoll.%s ();";
  uint querySize = sizeof(templateQuery) + strlen(tableID);
  char *query = salloc(querySize);
  sprintf(query, templateQuery, tableID);
  int s = runSql(ctx, query);
  free(query);
  return s;
};

/**
 * @brief  Load, parse and Execute the sqlTemplate file against postgres using psql interface
**/
int sqlImportCsv(_sqlCtx *ctx){
  assert(ctx);
  char templateQuery[] = "COPY modbuspoll.%s FROM '%s' DELIMITER ',' CSV HEADER;";
  char *csvFile = ctx->inoutFile.filePath; /* Per process filename */
  assert(csvFile);
  uint querySize = sizeof(templateQuery) + strlen(ctx->table) + strlen(csvFile);
  char *query = salloc(querySize);
  sprintf(query, templateQuery, ctx->table, csvFile); /* Set table/file for COPY query */
  int s = runSql(ctx, query);
  free(query);
  return s;
};

char *insertCsvHeader(_ln *deviceData){
    _ln *data = deviceData;
    char *csvHeader = salloc_init(_csv_timestamp_header_); /* Default column for all devices */
    char *csvColumn = salloc_init(_csv_timestamp_header_);
    char *columnID = salloc_init(data->data->key);
    while(data){ 
      char colDelimiter = ( data->next == NULL ) ? '\n' : _csv_std_delimiter_; /* Last row data? */
      columnID = srealloc_copy(columnID, data->data->key);
      csvColumn = srealloc( csvColumn, strlen(columnID) + _csv_delimiter_size_);
      sprintf(csvColumn, "%s%c", columnID, colDelimiter);
      int headerNewSize = strlen(csvHeader) + strlen(csvColumn);
      csvHeader = srealloc( csvHeader, headerNewSize);
      strcat(csvHeader, csvColumn);
      data = data->next;  
    } 
    free(columnID);
    free(csvColumn);
    assert(csvHeader);
    return csvHeader;
};

char *appendCsvData(_ln *deviceData, char *row){
    /* Insert timestamp with timezone as first column */
  assert(deviceData && row);
  char *tmz = timestampz();
  int tmzSize = strlen(row) + strlen(tmz) + _csv_delimiter_size_;
  row = srealloc(row, tmzSize);
  char *column = salloc(tmzSize);
  sprintf(column, "%s%c", tmz, _csv_std_delimiter_); 
  strcat(row, column);
  assert(row);
  _ln *currData = deviceData;
  assert(currData);
  while(currData){ /* Insert device's data */
    char delimiter = ( currData->next == NULL ) ? '\n' : _csv_std_delimiter_; /* Last row data? */
    char *data = currData->data->value;   /* Data */
    assert(data);
    char *dataType = currData->data->next->value; 
    assert(dataType);
    int colSize = strlen(data) + _csv_delimiter_size_;
    char *dataTemp = salloc(_byte_size_); /* initialize */
    if(strcmp(dataType, _pgsql_varchar_) == 0){ 
      colSize+= _csv_quote_size_;
      row = srealloc(row, strlen(row) + colSize);
      dataTemp = srealloc( dataTemp, colSize);
      sprintf(dataTemp, "%c%s%c%c", _csv_squote, data, _csv_squote, _csv_std_delimiter_);
    }
    else{
      row = srealloc(row, strlen(row) + colSize);
      dataTemp = srealloc( dataTemp, colSize);
      sprintf(dataTemp, "%s%c", data, delimiter);
    }
    strcat(row, dataTemp);
    currData = currData->next;
    free(dataTemp);
  } /* Insert device's data */
  free(tmz);
  free(column);
  return row;
};

_sqlCtx *persistData(_ln *deviceData, _ln *deviceConfig){
  assert(deviceData && deviceConfig);
  static double dTime = _start_;
  static char *dataBuffer;
  static _sqlCtx *sqlCtx;
  static int once = true;/* Try once to create devices table */
  if(once){
    once = false;
    sqlCtx = sqlCtxInit(sqlCtx, deviceConfig, deviceData);
  }
  if(dTime == _start_){ /* Start bufferring device data */
    cpu_time(_start_);
    //sqlCtx = sqlCtxInit(sqlCtx, deviceConfig, deviceData);
    _ln *dataAvaliable = deviceData;
    char *csvHeader = insertCsvHeader(dataAvaliable);
    dataBuffer = salloc_init(csvHeader);
    free(csvHeader);
  } /* Start bufferring device data */
  dTime = cpu_time(_check_);
  if( dTime > sqlCtx->inoutFile.persist_dt ) { /* Dump/Store buffered device data */
    FILE *outputFile = fopen(sqlCtx->inoutFile.fileName, "w+");
    if(outputFile == NULL)
      return NULL;
    int outFileWritten = fprintf(outputFile, "%s", dataBuffer); 
    fclose(outputFile);
    if (outFileWritten){
      sqlImportCsv(sqlCtx);  /* import dumped data to postgres using psql */
#ifndef QUIET_OUTPUT
      printf("Info: Buffered data saved\n");
#endif
    } 
    remove(sqlCtx->inoutFile.fileName); 
    //sqlCtxFree(sqlCtx);
    free(dataBuffer);
    dTime = cpu_time(_start_);
    return sqlCtx;
  }  /* Dump/Store buffered device data */
  dataBuffer = appendCsvData(deviceData, dataBuffer);
  assert(dataBuffer);
  return sqlCtx;
};