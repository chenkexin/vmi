#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
int main (int argc,char ** argv)
{
   /* setuid(12345);
    system("/bin/sh");*/
    open("/home/chenkx/ptrace.h", O_RDONLY);
    /*int a = 0;
    int b = 3;
    int c = b/a;*/
   printf("hhhhh\n");
     return 0;
}
