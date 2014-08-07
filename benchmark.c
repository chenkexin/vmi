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
//printf("delta: %ld\n", (ed - st));
return (ed - st);
}

int main(int argc, char* argv[])
{
    struct timeval start;
    struct timeval mid1;
    struct timeval mid2;
    struct timeval mid3;
    struct timeval end;

    long int t_open = 0;
    long int t_read = 0;
    long int t_write = 0;
    long int t_close = 0;
    long int t_create = 0;
    long int t_rename = 0;
    long int t_chmod = 0;
    long int t_remove = 0;
    long int t_mkdir = 0;
    long int t_rmdir = 0;
    long int t_link = 0;
    long int t_unlink = 0;
    long int t_getgid = 0;
    long int t_setgid = 0;
    long int t_setuid = 0;
    long int t_getuid = 0;
    long int t_getpid = 0;
    long int t_syslog = 0;
    long int t_time = 0;
    long int t_chown = 0;

    char buf1[] = "wow, such window~";
    char buf2[17];

    int fd_temp;

    gid_t gid_temp;
    uid_t uid_temp;
    pid_t pid_temp;
    time_t time_temp;
    //---------------------correction for gettimeofday---------
    int i =0;
    for(i; i<1000; i++)
    {
        gettimeofday(&start,NULL);
        gettimeofday(&mid1, NULL);
    }
    //--------------------- create -------------------
    i = 0; 
    for( i; i< 1000; i++)
    {
        gettimeofday(&start, NULL);
        fd_temp = creat("/tmp/test1.dat", O_CREAT);
        gettimeofday(&mid1, NULL);
        rename("/tmp/test1.dat", "/tmp/test2.dat");
        gettimeofday(&mid2, NULL);
        chmod("/tmp/test2.dat", S_ISGID);
        gettimeofday(&mid3, NULL);
        
        remove("/tmp/test1.dat");
        gettimeofday(&end, NULL);
        
        t_create += check_time( &start, &mid1);
        t_rename += check_time( &mid1, &mid2);
        t_chmod += check_time(&mid2, &mid3);
        t_remove += check_time(&mid3, &end);
    }
    printf("t_reanme: %ld\n", t_rename);   
    printf("t_chmod: %ld\n", t_chmod);   
    printf("t_remove: %ld\n", t_remove);   

    //---------------- open read write close -------------------
    i = 0;
    for(i; i < 1000; i ++)
    {
        gettimeofday(&start,NULL);
        fd_temp = open("/tmp/test.dat", O_CREAT|O_RDWR);  
        
        gettimeofday(&mid1, NULL);
        write( fd_temp, buf1, 17);        
        
        gettimeofday(&mid2, NULL);
        read( fd_temp, buf2, 17);
        
        gettimeofday(&mid3, NULL);
        close(fd_temp);
        gettimeofday(&end, NULL);
        
        t_open += check_time(&start, &mid1);
        t_write += check_time(&mid1, &mid2);
        t_read += check_time(&mid2, &mid3);
        t_close += check_time(&mid3, &end);
    }
    printf("t_open: %ld\n", t_open);   
    printf("t_write: %ld\n", t_write);   
    printf("t_read: %ld\n", t_read);   
    printf("t_close: %ld\n", t_close);   
    printf("t_create: %ld\n", t_create); 

    //----------------- mkdir rmdir link unlink -------- 
    i = 0;
    for( i; i<1000; i++)
    {
        gettimeofday(&start,NULL);    
        mkdir("/tmp/test", S_IRWXU);
        gettimeofday(&mid1, NULL);
        rmdir("/tmp/test");
        gettimeofday(&mid2, NULL);
        link("/tmp/test1.dat","/tmp/test2");
        gettimeofday(&mid3, NULL);
        unlink("/tmp/test2");
        gettimeofday(&end, NULL);

        t_mkdir += check_time(&start, &mid1);
        t_rmdir += check_time(&mid1, &mid2);
        t_link += check_time(&mid2, &mid3);
        t_unlink += check_time(&mid3, &end);
    } 

    printf("t_mkdir: %ld\n", t_mkdir);   
    printf("t_rmdir: %ld\n", t_rmdir);   
    printf("t_link: %ld\n", t_link);   
    printf("t_unlink: %ld\n", t_unlink);   

    //-------------------- getuid setuid getgid setgid-----
    i = 0;
    for( i; i<1000; i++)
    {
        gettimeofday(&start, NULL);
        gid_temp = getgid();
        gettimeofday(&mid1, NULL);
        uid_temp = getuid();
        gettimeofday(&mid2, NULL);
        setgid(gid_temp);
        gettimeofday(&mid3, NULL);
        setuid(uid_temp);
        gettimeofday(&end, NULL);

        
        t_getgid += check_time(&start, &mid1);
        t_getuid += check_time(&mid1, &mid2);
        t_setgid += check_time(&mid2, &mid3);
        t_setuid += check_time(&mid3, &end);
    }
    printf("t_getgid: %ld\n", t_getgid);   
    printf("t_getuid: %ld\n", t_getuid);   
    printf("t_setgid: %ld\n", t_setgid);   
    printf("t_setuid: %ld\n", t_setuid);  

    //--------------------getpid syslog chown time----------
    openlog ("/tmp/testlog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    i = 0;
    for( i; i<1000; i++)
    {
        gettimeofday(&start, NULL);
        pid_temp = getpid();
        gettimeofday(&mid1, NULL);
        syslog (LOG_INFO, "Wow~\t");
        gettimeofday(&mid2, NULL);
        chown("/tmp/testlog", 0,0);
        gettimeofday(&mid3, NULL);
        time_temp = time();
        gettimeofday(&end, NULL);         
        
        t_getpid += check_time(&start, &mid1);
        t_chown += check_time(&mid1, &mid2);
        t_syslog += check_time(&mid2, &mid3);
        t_time += check_time(&mid3, &end);
    }
 
    printf("t_getpid: %ld\n", t_getpid);   
    printf("t_syslog: %ld\n", t_syslog);   
    printf("t_time: %ld\n", t_time);   
    printf("t_chown: %ld\n", t_chown);  
}
