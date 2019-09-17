#include "head.h"

/* 执行shell内建命令 */
int builtin_command(char **argv)
{
    if(!strcmp(argv[0],"&")) return 1;//忽略单个&

    if(!strcmp(argv[0],"quit")){
        quit_command();
    }
    if(!strcmp(argv[0],"job")){
        printjobs();
        return 1;
    }
    if(!strcmp(argv[0],"bg")){
        bg_command(argv);
        return 1;
    }
    if(!strcmp(argv[0],"fg")){
        fg_command(argv);
        return 1;
    }
    return 0;
}

static int extractjid_pid(char **argv,jid_t *jidp,pid_t *pidp){
    char *p;
    if(argv[1]==NULL){
        printf("Usage: %s (<pid>|%%<jid>)\n>",argv[0]);
        return 0;
    }else p=argv[1];

    if(*p=='%'){
        p++;
        *jidp=(jid_t)atol(p);
        if(getjob_state(*jidp)==IDEL) {
            printf("%d: No such job\n>",*jidp);
            return 0;
        }
        *pidp=getpid_by_job(*jidp);
    }else{
        *pidp=(pid_t)atol(p);
        *jidp=getjob_by_pid(*pidp);
        if(*jidp==0){
            printf("%d: No such process\n>",*pidp);
            return 0;
        }
    }

    return 1;
}

void bg_command(char **argv)
{
    jid_t jid;
    pid_t pid;
    if(!extractjid_pid(argv,&jid,&pid)) return;

    char buf[MAXBUF];
    sigset_t prev_all;
    sigprocmask(SIG_BLOCK,getmask_all(),&prev_all);

    if(getjob_state(jid)==STOPED){
        kill(pid,SIGCONT);
        setjob_state(jid,RUNNING);
    }
    printjob(jid);

    sigprocmask(SIG_SETMASK,&prev_all,NULL);
}

void fg_command(char **argv)
{
    jid_t jid;
    pid_t pid;
    if(!extractjid_pid(argv,&jid,&pid)) return;

    sigset_t prev_all;
    sigprocmask(SIG_BLOCK,getmask_all(),&prev_all);

    give_terminalcontrol_to(pid);
    setfgjob(jid);
    if(getjob_state(jid)==STOPED){
        setjob_state(jid,RUNNING);
        kill(pid,SIGCONT);
    }
    
    while(fgjob_exist()) sigsuspend(&prev_all);//shell开始阻塞，直到收到前台进程的SIGCHLD

    sigprocmask(SIG_SETMASK,&prev_all,NULL);
}

void quit_command()
{
    exit(0);
}

void job_command()
{
    printjobs();
}