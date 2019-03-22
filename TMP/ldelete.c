/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *   ldelete  --  delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */
SYSCALL ldelete(int ldes)
{
        STATWORD ps;
        int     pid,i,prev;
        struct  lentry  *lptr;

        disable(ps);
        if (isbadlock(ldes) || locktab[ldes].lstate==LFREE) {
                restore(ps);
                return(SYSERR);
        }
        lptr = &locktab[ldes];
        lptr->lstate = LFREE;
	for(i=0;i<50;i++)
	{
		if(lptr->dprocs[i] ==1 )
		{ 
                        prev= q[lptr->lqtail].qprev;
                        while(prev!=lptr->lqhead)
                                {
                                        if(proctab[prev].pstate = PRWAIT)
                                        {
                                                lptr->dprocs[i]= -8;
                                        }
                                        else
                                        {
                                                lptr->dprocs[i] = -9;
                                        }
                                prev = q[prev].qprev;
                                }
                        }
                }
        if (nonempty(lptr->lqhead)) {
                while( (pid=getfirst(lptr->lqhead)) != EMPTY)
                  {
                    proctab[pid].pwaitret = DELETED;
		    ready(pid,RESCHNO);
                  }
                resched();
        }
        restore(ps);
        return(OK);
}
