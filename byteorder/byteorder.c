//byteorder.c
#include "unp.h"

int main(int argc, char **argv)
{
   union{
     short source;
     char cont[sizeof(short)];
   } un;
   
   un.source = 0x0102;
   printf("%s: ", CPU_VENDOR_OS);
   if(sizeof(short) == 2){
     if(un.cont[0] == 1 && un.cont[1] == 2)
         printf("big-endian\n");
     else if(un.cont[0] == 2 && un.cont[1] == 1)
         printf("little-endian\n");
     else
         printf("unknown\n");
   } else
         printf("sizeof(short) = %lu\n", sizeof(short));
   exit(0);
}
