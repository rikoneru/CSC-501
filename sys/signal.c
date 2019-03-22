/* signal.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * signal  --  signal a semaphore, releasing one waiting process
 *------------------------------------------------------------------------
 */

extern int ctr1000;
SYSCALL signal(int sem)
{
	STATWORD ps;    
	register struct	sentry	*sptr;
	int time,prev;

	disable(ps);
	if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
		restore(ps);
		return(SYSERR);
	}
	if ((sptr->semcnt++) < 0)
	{
		ready(prev=getfirst(sptr->sqhead), RESCHYES);
		time= ctr1000 - proctab[prev].pltime;
		kprintf(" SEM:  \n The writer with priority 60 has been waiting in the queue for %d",time );
	}
	restore(ps);
	return(OK);
}
