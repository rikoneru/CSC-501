/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	struct lentry *lptr;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
			if(proctab[pid].lockid!=0)
		{
			int ldes,prev,temp;
			ldes = proctab[pid].lockid;
			lptr = &locktab[ldes];
			if(proctab[pid].pprio >= lptr ->lprio)
			{
				dequeue(pid);
				prev= q[lptr->lqtail].qprev;
				lptr->lprio = 0;
				while(prev!=lptr->lqhead)
				{
					temp = prev;
					if(lptr->lprio<proctab[temp].pprio)
					{
						lptr->lprio = proctab[temp].pprio;	
					}
				prev = q[prev].qprev;
				}
			}
			else
			{
				dequeue(pid);
			}
				update_holding_locks(ldes);
		}




	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}


void update_holding_locks(int ldes)
{
	struct lentry *lptr;
	lptr= &locktab[ldes];
	int i;
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
				if(proctab[i].pinh > lptr->lprio )
					{
						
						proctab[i].pprio = proctab[i].pinh;
					}
				else{
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
