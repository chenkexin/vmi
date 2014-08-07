/*

micro benchmark for system call

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <syslog.h>
/*
    Remains to refine:
    1. switch to /tmp dir and do the test
    2. Open different file each time?
    3. Correction for gettimeofday
    4. Correction for iteration

    5. mkdir and rmdir vary greatly: 6000 - 40000
*/


struct timeval;
long check_time(struct timeval* start, struct timeval* end)
{
/*
This function will calculate the time used according to &start and &end
Needs more correction
*/
// unit: micro second
long st = start->tv_sec * 1000000 + start->tv_usec;
long ed = end->tv_sec * 1000000 + end->tv_usec;
//printff("delta: %ld\n", (ed - st));
return (ed - st);
}

int main(int argc, char* argv[])
{
    struct timeval start;
    struct timeval mid1;
    struct timeval mid2;
    struct timeval mid3;
    struct timeval end;

    long int t_total = 0;

    char buf1[] = "wow, such window~";
    char buf2[17];

    int fd_temp;

    gid_t gid_temp;
    uid_t uid_temp;
    pid_t pid_temp;

    openlog("/tmp/testlog", LOG_CONS| LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    //--------------------- create -------------------
    int i = 0; 
gettimeofday(&start, NULL);
    for( i; i< 1000; i++)
    {
        fd_temp = creat("/tmp/test1.dat", O_CREAT);
        rename("/tmp/test1.dat", "/tmp/test2.dat");
        chmod("/tmp/test2.dat", S_ISGID);
        remove("/tmp/test1.dat");
        fd_temp = open("/tmp/test.dat", O_CREAT|O_RDWR);  
        write( fd_temp, buf1, 17);        
        read( fd_temp, buf2, 17);
        close(fd_temp);
        mkdir("/tmp/test", S_IRWXU);
        rmdir("/tmp/test");
        link("/tmp/test1.dat","/tmp/ggg");
        unlink("/tmp/ggg");
        gid_temp = getgid();
        uid_temp = getuid();
        setgid(gid_temp);
        setuid(uid_temp);
        pid_temp = getpid();
        syslog (LOG_INFO, "Wow~\t");
        chown("/tmp/testlog", 0,0);
        
    }
        gettimeofday(&end, NULL); 
    t_total = check_time(&start, &end); 
    printf("t_total: %ld\n", t_total);  
}
