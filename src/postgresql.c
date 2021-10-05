
#include "postgresql.h"

#define _cache_size ((uint)2) /* Rows to be stored in memory before persist */

_sqlCtx *sqlCtxInit(_sqlCtx *sqlCtx, char* deviceID){
  assert(deviceID);
  sqlCtx = (_sqlCtx*)calloc(sizeof(_sqlCtx), _byte_size_);
  assert(sqlCtx);
  pid_t pid = getpid();
  sqlCtx->pid = salloc(str_digits(pid)); 
  sprintf(sqlCtx->pid, "%d", pid);
  assert(sqlCtx->pid);
  sqlCtx->hostname = salloc_init(_pgsql_host_);
  sqlCtx->port     = _pgsql_port_;
  sqlCtx->auth     = salloc_init(_mbpoll_auth_);
  sqlCtx->user     = salloc_init(_mbpoll_user_);
  sqlCtx->database = salloc_init(_mbpoll_database_);
  sqlCtx->schema   = salloc_init(_mbpoll_schema_);
  sqlCtx->table    = salloc(strlen(_mbpoll_table_) + _byte_size_ +strlen(deviceID));
  sprintf(sqlCtx->table, "%s_%s", deviceID, _mbpoll_table_); /* deviceID_modbuspoll */
  /* Template SQL script to be executed agaist postgres */
  sqlCtx->sqlTemplate = (char*)salloc(strlen(_sql_template_copy_) + strlen(_mbpoll_sqlDir_));
  sprintf(sqlCtx->sqlTemplate, "%s%s", _mbpoll_sqlDir_, _sql_template_copy_);
  /* File used to export data */
  sqlCtx->inoutFile.cacheSize = _cache_size;
  sqlCtx->inoutFile.fileName = salloc(strlen(sqlCtx->pid) + _byte_size_ + strlen(_csv_file_));
  sprintf(sqlCtx->inoutFile.fileName, "%s_%s", sqlCtx->pid, _csv_file_); /* 12345678_modbuspoll.csv */
  sqlCtx->inoutFile.filePath = salloc(strlen(_mbpoll_dataDir_) + strlen(sqlCtx->inoutFile.fileName));
  sprintf(sqlCtx->inoutFile.filePath, "%s%s", _mbpoll_dataDir_, sqlCtx->inoutFile.fileName); 
  return (sqlCtx != NULL ? sqlCtx : NULL);
};

int sqlCtxFree(_sqlCtx *sqlCtx){
  assert(sqlCtx);
  free(sqlCtx->sqlTemplate);
  free(sqlCtx->table);
  free(sqlCtx->schema);
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

/**
 * @brief  Load, parse and Execute the sqlTemplate file against postgres using psql interface
**/
int runSql(_sqlCtx *ctx){
  assert(ctx);
  FILE *templateFile = fopen(ctx->sqlTemplate, "r");
  assert(templateFile);
  char *templateQuery = salloc(_sql_template_line_); /* Initialize vector */
  char *line = salloc(_sql_template_line_);
  while( fgets(line, _sql_template_line_, templateFile) ){ /* Load sql template file */
    int templateNewSize = strlen(templateQuery) + strlen(line);
    templateQuery = srealloc(templateQuery, templateNewSize);
    strcat(templateQuery, line);
  };
  char *csvFile = ctx->inoutFile.filePath; 
  assert(csvFile);

  uint querySize = strlen(templateQuery) + strlen(ctx->table) + strlen(csvFile);
  int tagSize = 2; 
  querySize -= tagSize * 2; /* The 2 tags %s will be substituted by values*/
  char *query = salloc(querySize);
  sprintf(query, templateQuery, ctx->table, csvFile); /* Set table/file for COPY query */
  char *auth = salloc_init(ctx->auth);
#ifndef NDEBUG  
  char *psql = (char*)"psql";
#else 
  char *psql = (char*)"psql -q";  /* Be quiet */
#endif
  char *user = salloc_init(ctx->user);
  char *hostname = salloc_init(ctx->hostname);
  char *port = salloc(str_digits(ctx->port));
  sprintf(port, "%d", ctx->port);
  char *database = salloc_init(ctx->database);
  char *cmdTemplate = (char*)"%s %s --single-transaction --host=%s --port=%s --dbname=%s --username=%s --command=%s ";
  uint cmdSize = strlen(auth) + strlen(psql) + strlen(hostname) + 
                 strlen(port) + strlen(database) + strlen(user) + 
                 strlen(query) + strlen(cmdTemplate);
  cmdSize -= tagSize * 7;
  char *cmd = (char*)salloc(cmdSize);
  sprintf(/*Output  */ cmd, 
          /*Template*/ cmdTemplate, 
          /*Values  */ auth, psql, hostname, port, database, user, query);
  int s = system(cmd); /* run query */
  
  free(cmd);
  free(database);
  free(port);
  free(hostname);
  free(user);
  free(auth);
  free(query);
  free(line);
  free(templateQuery);
  return s;
};

char *insertCsvHeader(_ln *deviceData){
    _ln *data = deviceData;
    char *csvHeader = salloc_init(_csv_timestamp_header_); /* Default column for all devices */
    char *csvColumn = salloc_init(_csv_timestamp_header_);
    char *columnID = salloc_init(data->data->key);
    while(data){ 
      char colDelimiter = ( data->next == NULL ) ? '\n' : _csv_std_delimiter_; /* Last row data? */
      columnID = srealloc(columnID, strlen(data->data->key));
      strcpy(columnID, data->data->key);
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
  return (row != NULL ? row : NULL );
};

int persistData(char *deviceID, _ln *deviceData){
  assert(deviceID && deviceData);
  static u_int32_t cache = 0;
  static char *dataBuffer;
  static _sqlCtx *sqlCtx;
  if(cache == 0){ /* Start bufferring device data */
    sqlCtx = sqlCtxInit(sqlCtx, deviceID);
    _ln *dataAvaliable = deviceData;
    char *csvHeader = insertCsvHeader(dataAvaliable);
    dataBuffer = salloc_init(csvHeader);
    free(csvHeader);
  } /* Start bufferring device data */
  cache++;
  if( cache > sqlCtx->inoutFile.cacheSize ) { /* Dump/Store buffered device data */
    cache = 0;
    FILE *outputFile = fopen(sqlCtx->inoutFile.fileName, "w+");
    assert(outputFile);
    int outFileWritten = fprintf(outputFile, "%s", dataBuffer); 
    fclose(outputFile);
    if (outFileWritten){
      runSql(sqlCtx);  /* import dumped data to postgres using psql */
#ifndef NDEBUG
      printf("Info: Buffered data saved\n");
#endif
    } 
    remove(sqlCtx->inoutFile.fileName);
    sqlCtxFree(sqlCtx);
    free(dataBuffer);
    return 0;
  }  /* Dump/Store buffered device data */
  dataBuffer = appendCsvData(deviceData, dataBuffer);
  assert(dataBuffer);
  return 0;
};