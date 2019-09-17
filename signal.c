#include "head.h"
static sigset_t mask_all,mask_sigchld,mask_sigchld_sigpg,mask_sigpg_ex;

sigset_t* getmask_all(){return &mask_all;}
sigset_t* getmask_sigpg_ex(){return &mask_sigpg_ex;}

/* 信号处理函数，主要处理SIGCHLD */
void handler(int signum){

    int olderrno=errno,status;
    errno=0;
    sigset_t prev_all;
    char msg[100];
    pid_t pid;

    if(signum==SIGCHLD){
        while((pid=waitpid(-1,&status,WNOHANG|WUNTRACED))>0){//posix标准中，子进程停止时也向父进程发送SIGCHLD信号
            sigprocmask(SIG_BLOCK,getmask_all(),&prev_all);
            jid_t jid=getjob_by_pid(pid);
            if(jid==getfgjob()){
                give_terminalcontrol_to_shell();
                clearfgjob();
            }
            
            if(WIFSTOPPED(status)){
                setjob_state(jid,STOPED);
                sprintf(msg,"\nJob [%d] %d stopped by Signal %s\n>",jid,pid,strsignal(WSTOPSIG(status)));
            }else if(WIFSIGNALED(status)){
                deletejob(jid);
                sprintf(msg,"\nJob [%d] terminated by Signal %s\n>",jid,strsignal(WTERMSIG(status)));
            }else if(WIFEXITED(status)){
                deletejob(jid);
                if(WEXITSTATUS(status)==0) sprintf(msg,"Job [%d] terminated\n>",jid);
                else goto unblock;
            }

            Sio_puts(msg);
            unblock:sigprocmask(SIG_SETMASK,&prev_all,NULL);
        }
        if(errno&&errno!=ECHILD) unix_error("waitpid error");
    }
    errno=olderrno;
}

/* 默认的signal语义,增加可移植性 */
handler_t *Signal(int signum,handler_t *handler)
{
    struct sigaction action,old_action;

    action.sa_handler=handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags=SA_RESTART;

    if(sigaction(signum,&action,&old_action)<0)//signation默认把当前signum加入到mask中，除非设置标志
        unix_error("Signal error");
    return (old_action.sa_handler);
}

void unix_error(char *msg){
    fprintf(stderr,"%s: %s\n",msg,strerror(errno)); 
    exit(0);
}

void initsignal()
{
    sigfillset(&mask_all);

    sigfillset(&mask_sigpg_ex);
    sigdelset(&mask_sigpg_ex,SIGPG);
}