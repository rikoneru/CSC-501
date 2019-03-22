/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	struct  lentry  *lptr;
	int ldes, max, k;
	pptr = &proctab[pid];

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if(pptr->pinh!=0)
	{
		pptr->pinh= newprio;
		chprio2(pid,newprio);
		restore(ps);
		return(newprio);
	}
	else{
	pptr->pprio = newprio;
	if(proctab[pid].pstate == PRWAIT && proctab[pid].lockid !=0)
	{
		ldes = proctab[pid].lockid;
		lptr = &locktab[ldes];
		check_all_locks(pid);
		update_lprio2(ldes);
		update_holding_locks3(ldes);
	}
	if(proctab[pid].lockid == 0)
	{
		check_all_locks(pid);
	}		
	restore(ps);
	return(newprio);
	}
}


void update_lprio2(int ldes){
	struct lentry *lptr;
	lptr= &locktab[ldes];
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

void check_all_locks(int pid)
{
	int i,max=0,l;
	int k=0;
	for(i=0;i<50;i++)
		{
			if(locktab[i].hprocs[pid]== READ || locktab[i].hprocs[pid]== WRITE )
			{
			max = update_holding_locks2(pid,i);
			}
			if(k<max)
			{
				k = max;
			}
		}
		if(k!=0 && proctab[pid].pprio<k)
		{
			if(proctab[pid].pinh ==0)
			{
				proctab[pid].pinh = proctab[pid].pprio;
			}
			if(proctab[pid].pinh > k )
			{	
				l= proctab[pid].pinh;
				proctab[pid].pinh=0;
				chprio2(pid,l);
				
			}
			else
			{
				chprio2(pid,k);
			}
		}
}

int update_holding_locks2(int pid, int ldes)
{
	struct lentry *lptr;
	lptr= &locktab[ldes];
	int max=0;
	if(proctab[pid].pprio< lptr->lprio)
		{
			max = lptr-> lprio;
		}
	return(max);
}


void update_holding_locks3(int ldes)
{
	struct lentry *lptr;
	lptr= &locktab[ldes];
	int i,l;
	for(i=0;i<50;i++)
	{
		if(lptr->hprocs[i]== READ || lptr->hprocs[i]== WRITE)
		{
				if(lptr-> lprio !=0)
				{
				if(proctab[i].pinh==0)
					{
						proctab[i].pinh = proctab[i].pprio;
					}
				if(proctab[i].pprio < lptr->lprio)
					{
						chprio2(i,lptr->lprio);
					}
				
					if(proctab[i].pinh > lptr->lprio )
					{
						l= proctab[i].pinh;
						proctab[i].pinh=0;
						chprio2(i,l);
						proctab[i].pinh=0;
						 
					}
				else if(proctab[i].pprio< lptr-> lprio)
				{
					chprio2(i,lptr->lprio);
				}
	
			}
			else{
				if(proctab[i].pinh!=0)
				{
					proctab[i].pprio = proctab[i].pinh;
				}
			}
		}
	}
}
