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
SYSCALL chprio2(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	struct  lentry  *lptr;
	int ldes;
	pptr = &proctab[pid];

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	if(proctab[pid].pstate == PRWAIT && proctab[pid].lockid !=0)
	{
		ldes = proctab[pid].lockid;
		lptr = &locktab[ldes];
		check_all_locks22(pid);
		update_lprio22(ldes);
		update_holding_locks33(ldes);
	}
	if(proctab[pid].lockid == 0)
	{
		check_all_locks22(pid);
	}	
	restore(ps);
	return(newprio);
	}


void update_lprio22(int ldes){
	struct lentry *lptr;
	lptr= &locktab[ldes];
	int prev= q[lptr->lqtail].qprev;
    lptr->lprio =0;
	while(prev!=lptr->lqhead)
	{
		if(lptr->lprio<proctab[prev].pprio)
		{
			lptr->lprio = proctab[prev].pprio;	
		}
	prev = q[prev].qprev;
	}
}

void check_all_locks22(int pid)
{
	int i,max=0,l;
	int k=0;
	for(i=0;i<50;i++)
		{
			if(locktab[i].hprocs[pid]== READ || locktab[i].hprocs[pid]== WRITE )
			{
			max = update_holding_locks22(pid,i);
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

int update_holding_locks22(int pid, int ldes)
{
	struct lentry *lptr;
	lptr= &locktab[ldes];
	int i, max;
	if(proctab[pid].pprio< lptr->lprio)
		{
			max = lptr-> lprio;
		}
	return(max);
}


void update_holding_locks33(int ldes)
{
	struct lentry *lptr;
	lptr= &locktab[ldes];
	int i,x,l;
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
			else
            {
				if(proctab[i].pinh!=0 && proctab[i].pinh> proctab[i].pprio)
				{
                    x= proctab[i].pinh;
                    proctab[i].pinh =0;
                	chprio2(i,x);
				}
			}
		}
	}
}
