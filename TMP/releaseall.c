#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

extern struct lentry locktab[];
extern int  ctr1000;
int releaseall(int numlocks,int ldes1,...)
{
STATWORD ps;
int ldes,i;
int flag=0;
disable(ps);
struct lentry *lptr;
for(i=0;i<numlocks;i++)
{
	ldes = (int)*((&ldes1) + i);
	lptr = &locktab[ldes];
	
	if(isbadlock(ldes) || lptr -> lstate == LFREE || lptr -> hprocs[currpid] == LNONE)
	{
		flag =1;
	}
	if(lptr -> hprocs[currpid] == READ)
	{
		lptr -> nreaders --;
		if(lptr->nreaders >0){
		lptr->hprocs[currpid] = LNONE;
		update_inheritance(currpid);
		restore(ps);
		return(OK);}
		else
		{
			lptr->hprocs[currpid] = LNONE;
			update_inheritance(currpid);
			lptr->hprocs[currpid] = READ;
			get_process(ldes);
		}
	}
	if(lptr -> hprocs[currpid] == WRITE)
	{
		/* Write is about be released") */
		lptr->hprocs[currpid] = LNONE;
		update_inheritance(currpid);
		lptr->hprocs[currpid] = WRITE;
		get_process(ldes);	
	}

}
resched();
if(flag==1)
{
	restore(ps);
	return(SYSERR);
}
restore(ps);
return(OK);

}

int get_process(int ldes)
{
struct lentry *lptr;
int time1,time2;
lptr = &locktab[ldes];
if(q[lptr->lqtail].qprev == lptr->lqhead)
	{
		lptr->ltype = LNONE;
		return(OK);
	}
int prev  = q[lptr->lqtail].qprev;
if(lptr -> hprocs[currpid] == READ)
{
	lptr -> ltype = proctab[prev].locktype;
	lptr -> hprocs[prev] = proctab[prev].locktype;
	lptr->hprocs[currpid] = LNONE;
	proctab[prev].lockid = 0;
	time1 = ctr1000- proctab[prev].pltime;
	int temp= q[lptr->lqtail].qprev;
	dequeue(temp);
	ready(temp,RESCHNO);
	wakeup_update_inheritance(temp,ldes);
	return(OK);
}

if(lptr->hprocs[currpid] == WRITE)
  {
	if(proctab[prev].locktype == WRITE)
    {
		int time = proctab[prev].pltime;
	    int key= q[prev].qkey;
		int flag=0;
		lptr->ltype = proctab[prev].locktype;
		lptr->hprocs[currpid] = LNONE;
		prev=q[prev].qprev;
		int temp= q[lptr->lqtail].qprev;
			while(prev!=lptr->lqhead)
			{
				if(key == q[prev].qkey && proctab[prev].locktype == READ)
				{
					if(proctab[prev].pltime - time < 500)
					{
						flag =1;
						lptr -> ltype = proctab[prev].locktype;
						lptr-> hprocs[prev] = proctab[prev].locktype;
						temp = prev;
						prev= q[prev].qprev;
						proctab[temp].lockid = 0;
						dequeue(temp);
						ready(temp,RESCHNO);
						wakeup_update_inheritance(temp,ldes);
					}
					else
					{	
						prev = q[prev].qprev;
					}
				}
				else
				{
					prev = q[prev].qprev;
				}
			}		
		if(flag!=1)
			{
				lptr-> hprocs[temp] = proctab[temp].locktype;
						proctab[temp].lockid = 0;
						time2 = ctr1000- proctab[temp].pltime;
						kprintf(" The writer has waited for %d", time2);
						dequeue(temp);
                		ready(temp,RESCHNO);
						wakeup_update_inheritance(temp,ldes);
			}
	return(OK);
	 }
      if(proctab[prev].locktype == READ)
	{
		int flag =0;
		int temp,time;
		int key;
		lptr -> ltype = proctab[prev].locktype;
		lptr->hprocs[prev] = proctab[prev].locktype;
		lptr -> nreaders++;
		temp= prev;
		prev = q[prev].qprev;
		proctab[temp].lockid = 0;
		dequeue(temp);
		ready(temp,RESCHNO);
		//wakeup_update_inheritance(temp,ldes);
		while(prev!=lptr->lqhead)
		{
				if(proctab[prev].locktype == READ && flag == 0)
				{
					lptr->nreaders++;
					lptr -> ltype = proctab[prev].locktype;
					lptr-> hprocs[prev] = proctab[prev].locktype;
					temp = prev;
					prev= q[prev].qprev;
					proctab[temp].lockid = 0;
					dequeue(temp);
					ready(temp,RESCHNO);
					wakeup_update_inheritance(temp,ldes);
				}
				else if(proctab[prev].locktype == WRITE && flag == 0)
				{
					key = q[prev].qkey;
					flag=1;
					time = proctab[prev].pltime;
					prev = q[prev].qprev;
				}
				else if(proctab[prev].locktype == READ && flag == 1)
				{
					
					if(key == q[prev].qkey && proctab[prev].pltime - time >500)
					{
						lptr-> nreaders++;
						lptr -> ltype = proctab[prev].locktype;
						lptr -> hprocs[prev] = proctab[prev].locktype;
						temp=prev;
						prev = q[prev].qprev;
						dequeue(temp);
						ready(temp,RESCHNO);
						wakeup_update_inheritance(temp,ldes);
					}
					else { prev = q[prev].qprev;}
				}
		
				else if(proctab[prev].locktype == WRITE && flag == 1){ prev = q[prev].qprev;}
		}
		}
	}

}

void update_inheritance(currpid)
{
	int ldes2;
	if(proctab[currpid].pinh!=0)
		{ 
			proctab[currpid].pprio = proctab[currpid].pinh;
			ldes2= proctab[currpid].lockid;
			if(ldes2!=0)
			{
				check_all_locks22(currpid);
				update_lprio22(ldes2);
				update_holding_locks33(ldes2);

			}
			else
			{
				check_all_locks22(currpid);
			}
		}
}

void wakeup_update_inheritance(int temp, int ldes)
{
	update_lprio22(ldes);
	update_holding_locks33(ldes);
	check_all_locks22(temp);
}

