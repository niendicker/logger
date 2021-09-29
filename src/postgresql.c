
#include "postgresql.h"

int storePing(char *devID, int ping_ms){
  assert(devID);
  char script[] = "./scripts/insertPing.sh";
  char *cmd = (char*)malloc(200);
  sprintf(cmd, "%s %s %.02f &2>/dev/null &", script, devID, (float)(ping_ms/10E2));
  system(cmd);
  free(cmd);
  return 0;
};