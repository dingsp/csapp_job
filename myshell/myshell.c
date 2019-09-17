#include "head.h"
static pid_t shellpid;

/* 把终端给进程组 */
void give_terminalcontrol_to(pid_t pid){
    tcsetpgrp(0,pid);
}

void give_terminalcontrol_to_shell(){
    tcsetpgrp(0,shellpid);
}

/* 解析命令,构建参数数组(首先替换掉换行符) */
int parseline(char *buf,char **argv)
{
    char *delim;
    int argc;
    int bg;

    buf[strlen(buf)-1]=' ';//替换掉'\n'
    while (*buf&&(*buf==' '))//忽略掉开头的空格
        buf++;
    
    argc=0;
    while ((delim=strchr(buf,' '))){
        argv[argc++]=buf;
        *delim='\0';
        buf=delim+1;
        while (*buf&&(*buf==' ')) buf++;
    }
    argv[argc]=NULL;
    
    if(argc&&(bg=(*argv[argc-1]=='&'))!=0) argv[--argc]=NULL;
    
    return bg;
}

void eval(char *cmdline)
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;
    jid_t jid;

    strcpy(buf,cmdline);
    bg=parseline(buf,argv);
    if(argv[0]==NULL) return;//忽略空行
    if(!builtin_command(argv)){
        sigset_t prev_all;
        sigprocmask(SIG_BLOCK,getmask_all(),&prev_all);

        if((pid=fork())==0){//子进程
            sigprocmask(SIG_SETMASK,&prev_all,NULL);
            setpgid(0,0);
            kill(getppid(),SIGPG);
            if(execve(argv[0],argv,environ)<0){
                printf("%s: Command not found.\n>",argv[0]);
                exit(COMMANDNOTFOUND);
            }
        }else{//父进程
            jid=addjob(pid,cmdline);
            
            if(bg==0){//前台
                setfgjob(jid);
                sigsuspend(getmask_sigpg_ex());
                give_terminalcontrol_to(pid);//需要与子进程的setpgid()同步执行

                while(fgjob_exist()) sigsuspend(&prev_all);//直到收到前台作业的SIGCHLD才会结束循环
            }else//后台
                printjob(jid);
        }
        sigprocmask(SIG_SETMASK,&prev_all,NULL);
    }
}

void initshell()
{
    initjob();
    initsignal();

    shellpid=getpid();

    Signal(SIGCHLD,handler);
    Signal(SIGPG,handler);//sigsuspend需执行信号处理函数才能返回
    Signal(SIGINT,handler);//posix标准中，执行execve的子进程会继承SIG_IGN的信号处理函数，所以这里不能简单的忽略
    Signal(SIGTSTP,handler);
    Signal(SIGTTOU,handler);//使shell进程在后台时也能执行tcsetpgrp
}

/*
    main例程，不停的read/evaluate
    shell程序的主要功能就是管理程序和键盘输入的交互，一个时间只能有一个前台作业
 */
int main(){
    initshell();
    char cmdline[MAXLINE];
    printf("> ");
    
    while (1)
    {
        fgets(cmdline,MAXLINE,stdin);
        if(feof(stdin)) exit(0);

        eval(cmdline);
    }
}