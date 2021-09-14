
/**
 * @file modbusTcp.h
 * @author Marcelo Niendicker Grando
 * @date 2021/09/13
 * @brief File containing example of doxygen usage for quick reference.
 *
 * Load device configuration and registers map 
 * Do the transactions = queries to modbus slave device
 * 
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 */ 

//#define NDEBUG
#include "errorCtrl.h"
#include "modbusTcp.h"
int main(int argc , char *argv[])
{
   mbCtx* GC600 = mbInit("gc600.conf", "gc600.map");
   assert(GC600 != NULL);
   
   while(1){
      if(mbUpdate(GC600) == 0){
         for(int i=0; i < GC600->dev.mapSize; i++)
            printf("Register[%d]: %d \n", i, GC600->dev.map[i]->lastValid);
      }
      sleep(1);
   }
}

