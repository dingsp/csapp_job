/* $begin head.h */
#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sysexits.h>

/* Builtin command exec*/
int builtin_command(char **argv);
void job_command();
void quit_command();
void fg_command(char **argv);
void bg_command(char **argv);
#define COMMANDNOTFOUND 127

/* Singal synchronization*/
void handler(int signum);
sigset_t* getmask_all();
sigset_t* getmask_sigpg_ex();
void initsignal();

/* Parse command and shecule jobs */
void initshell();
void eval(char *cmdline);
int parseline(char *buf,char **argv);
int builtin_command(char **argv);
void echo(int connfd);
void give_terminalcontrol_to(pid_t pid);
void give_terminalcontrol_to_shell();

/* ***********************
 *  Job managment model
 * ************************/
#define MAXJOBS 100
#define MAXARGS 128

/* job struct*/
typedef unsigned int jid_t;
typedef unsigned char job_state; 
typedef struct  Job{
    pid_t pid;
    job_state jst;
    char *cmdlinep;
}Job;

/* job state related */
void setjob_state(jid_t jid,job_state state);
#define IDEL 0
#define STOPED 1
#define RUNNING 2
#define JOBSTATE(x) x==1?"Stopped":"Running"

/* job related*/
void initjob();
jid_t getjob_idel();
int getjob_state(jid_t jid);
pid_t getpid_by_job(jid_t jid);
jid_t getjob_by_pid(pid_t pid);
jid_t addjob(pid_t pid,char *cmdline);
void deletejob(jid_t jid);
void printjob();
void printjobs();

/* foreground job related */
#define SIGPG SIGUSR1
jid_t getfgjob();
void setfgjob(jid_t jid);
int fgjob_exist();
void clearfgjob();

/* Job managment model end */

/* External variables */
extern int h_errno;    /* Defined by BIND for DNS errors */ 
extern char **environ; /* Defined by libc */

/* Misc constants */
#define	MAXLINE	 8192  /* Max text line length */
#define MAXBUF   8192  /* Max I/O buffer size */

/* Our own error-handling functions */
void unix_error(char *msg);

/* Signal wrappers */
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/* Sio (Signal-safe I/O) routines */
ssize_t sio_puts(char s[]);
ssize_t sio_putl(long v);
void sio_error(char s[]);

/* Sio wrappers */
ssize_t Sio_puts(char s[]);
ssize_t Sio_putl(long v);
void Sio_error(char s[]);

#endif /* __CSAPP_H__ */
/* $end head.h */