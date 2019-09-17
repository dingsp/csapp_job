#include "head.h"
/*
    关于某一段代码是否存在竞态条件的讨论：
        是否有多个进程共同访问且至少有一个进程在写，以及代码区间内是否存在并行性
 */
#define ISIDELJOB(x) ((job[x].jst)==IDEL)
#define ISFGJOB(x) (x==fgjid)
static Job job[MAXJOBS];
static volatile sig_atomic_t fgjid;

void initjob()
{
    memset(job,0,sizeof(Job)*MAXJOBS);
    fgjid=0;
}

pid_t getpid_by_job(jid_t jid)
{
    if(jid<1||jid>MAXJOBS) return 0;
    return job[jid].pid;
}

jid_t getjob_by_pid(pid_t pid)
{
    for(int i=1;i<MAXJOBS;i++){
        if(job[i].jst!=IDEL&&job[i].pid==pid)
            return i;
    }
    return 0;
}

jid_t getjob_idel()
{
    for (jid_t i = 1; i < MAXJOBS; i++)
        if(ISIDELJOB(i))
            return i;
    return 0;
}

void setjob_state(jid_t jid,job_state state)
{
    job[jid].jst=state;
}

jid_t addjob(pid_t pid,char *cmdline)
{
    jid_t jid=getjob_idel();
    Job *p=&job[jid];
    if(jid!=0){
        p->pid=pid;
        p->jst=RUNNING;
        p->cmdlinep=malloc(sizeof(cmdline));
        strcpy(job[jid].cmdlinep,cmdline);
    }
    return jid;
}

void deletejob(jid_t jid)
{
    Job *p=&job[jid];
    p->jst=IDEL;
    p->pid=0;
    free(p->cmdlinep);
}

void printjobs()
{
    int flag=0;
    for (jid_t i = 1; i < MAXJOBS; i++){   
        if(!ISIDELJOB(i)&&!ISFGJOB(i)){
            printf("[%d] %d %-9s %s>",i,job[i].pid,JOBSTATE(job[i].jst),job[i].cmdlinep);
            flag=1;
        }
    }
    if(!flag) printf("No jobs\n>");
}

void printjob(jid_t jid)
{
    printf("[%d] %d %s>",jid,job[jid].pid,job[jid].cmdlinep);
}

int getjob_state(jid_t jid)
{   
    if(jid<1||jid>MAXJOBS) return -1;
    return job[jid].jst;
}

jid_t getfgjob(){ return fgjid; }

void setfgjob(jid_t jid){ fgjid=jid; }

void clearfgjob(){ fgjid=0; }

int fgjob_exist(){ return fgjid!=0; }