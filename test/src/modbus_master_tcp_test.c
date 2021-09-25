
#include <stdio.h>
#include <stdlib.h>     //malloc()
#include <unistd.h>     //read() and write()
#include <string.h>	   //strlen

#include "slave_config_parser.h"
//#include "modbusTcp.h"
int main(int argc , char *argv[])
{
   int i = 0;
   while(1){
   
   system("clear");
   printf("Counting...%d\n", i++);
   device GC600; //This is the guy
   char deviceConf[] = "gc600.conf";
   char deviceMap[] = "gc600.map";
   loadDeviceConf(&GC600, deviceConf);   
   loadDeviceMap(&GC600, deviceMap);
   assert(GC600.map[0] != NULL && GC600.mapSize >= 0);
   for(int i=0; i <= GC600.mapSize; i++){
      printf("%d %d %d", GC600.map[i]->function, GC600.map[1]->address, GC600.map[i]->size);
      printf("\n%s\n", GC600.map[i]->meaning);
   }

   while(GC600.mapSize >= 0){
      free(GC600.map[GC600.mapSize]);
      GC600.mapSize--;
   }
   usleep(100000);
   }
   return 0;
}

