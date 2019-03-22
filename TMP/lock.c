#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

extern struct lentry locktab[NLOCKS];
extern int  ctr1000;
int lock (int ldes1, int type, int priority)
{	
	STATWORD ps;
	disable(ps);
	struct lentry *lptr;
	lptr=&locktab[ldes1];
	if(lptr-> lstate == LFREE)
	{
		restore(ps);
		return(SYSERR);
	}
	if( lptr->dprocs[currpid] == -8)
	{
		restore(ps);
		return(DELETED);
	}
	if( lptr->dprocs[currpid] == -9)
	{
		restore(ps);
		return(SYSERR);
	}
	

	if(lptr-> lstate == LUSED && lptr->ltype == LNONE)
	{
		lptr -> ltype = type;
		lptr -> hprocs[currpid] = type;
		proctab[currpid].hlocks[ldes1] = type;
		proctab[currpid].locktype = type;
		if(lptr->ltype == READ)
			{
				lptr->nreaders++;
			}
		restore(ps);
		return(OK);
	}
	else if(lptr-> lstate == LUSED && lptr->ltype == READ)
	{
		if(type == READ){
		int prev=q[lptr->lqtail].qprev;
		while(prev!=lptr->lqhead)
		{
			if(q[prev].qkey>priority  && proctab[prev].locktype==WRITE)
			{
				get_inheritance(ldes1);
				proctab[currpid].pstate = PRWAIT;
				insert(currpid,lptr->lqhead,priority);
				proctab[currpid].pltime = ctr1000;
				proctab[currpid].hlocks[ldes1] = type;
				proctab[currpid].locktype = type;
				proctab[currpid].lockid = ldes1;
				update_lprio();
				resched();
				restore(ps);
				return(OK);
			}
			prev = q[prev].qprev;
		}
		lptr->nreaders++;
		proctab[currpid].locktype= type;
		lptr-> hprocs[currpid] = type;
		proctab[currpid].hlocks[ldes1] = type;
		proctab[currpid].locktype = type;
		set_inherit_lockgave(ldes1);
		restore(ps);
		return(OK);
			
	}
		if(type == WRITE){
			get_inheritance(ldes1);
			proctab[currpid].pstate = PRWAIT;
			insertd(currpid,lptr->lqhead,priority);
			proctab[currpid].pltime = ctr1000;
			proctab[currpid].hlocks[ldes1] = type;
			proctab[currpid].locktype = type;
			proctab[currpid].lockid = ldes1;
			update_lprio(ldes1);
			resched();
			restore(ps);
			return(OK);
		}
	}
	else if(lptr->lstate == LUSED && lptr->ltype ==WRITE)
	{
		get_inheritance(ldes1);
		proctab[currpid].pstate = PRWAIT;
		insertd(currpid,lptr->lqhead,priority);
		proctab[currpid].pltime = ctr1000;
		proctab[currpid].hlocks[ldes1] = type;
		proctab[currpid].locktype = type;
		proctab[currpid].lockid = ldes1;
		update_lprio(ldes1);
		resched();
		restore(ps);
		return(OK);
	}
restore(ps);
return(OK);
}

void get_inheritance(int ldes1)
{
	struct lentry *lptr;
	lptr= &locktab[ldes1];
	int i;
	for(i=0;i<50;i++)
				{
					if(lptr->hprocs[i]== READ || lptr->hprocs[i]== WRITE)
					{
						if(proctab[currpid].pprio > proctab[i].pprio)
						{
							if(proctab[i].pinh==0)
							{
								proctab[i].pinh = proctab[i].pprio;
							}
							chprio2(i,proctab[currpid].pprio);
						}
					}
				}
				
}

void set_inherit_lockgave(ldes1)
{
	struct lentry *lptr;
	lptr= &locktab[ldes1];
	int i;
	for(i=0;i<50;i++)
	{
		if(lptr->hprocs[i]== READ || lptr->hprocs[i]== WRITE)
		{
			if(proctab[i].pprio< lptr->lprio)
			{
				if(proctab[i].pinh==0)
					{
						proctab[i].pinh = proctab[i].pprio;
					}
				chprio2(i,lptr->lprio);

			}
		}
	}
}

void update_lprio(int ldes1){
	struct lentry *lptr;
	lptr= &locktab[ldes1];
	int prev= q[lptr->lqtail].qprev;
	lptr->lprio=0;
	while(prev!=lptr->lqhead)
	{
		if(lptr->lprio<proctab[prev].pprio)
		{
			lptr->lprio = proctab[prev].pprio;	
		}
	prev = q[prev].qprev;
	}
}