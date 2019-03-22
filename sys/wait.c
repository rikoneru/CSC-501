/* wait.c - wait */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  * wait  --  make current process wait on a semaphore
 *   *------------------------------------------------------------------------
 *    */
extern int ctr1000;
SYSCALL	wait(int sem)
{
	STATWORD ps;    
	struct	sentry	*sptr;
	struct	pentry	*pptr;

	disable(ps);

	if (isbadsem(sem)){
		restore(ps);
		return(SYSERR);
	}

	if((sptr= &semaph[sem])->sstate==SFREE){
		pptr = &proctab[currpid];
		restore(ps);
		if(pptr->pwaitret == DELETED)
			return(DELETED);
		else return (SYSERR);
	}
	
	if (--(sptr->semcnt) < 0) {
		(pptr = &proctab[currpid])->pstate = PRWAIT;
		pptr->pltime=ctr1000;
		pptr->psem = sem;
		enqueue(currpid,sptr->sqtail);
		pptr->pwaitret = OK;
		resched();
		restore(ps);
		return pptr->pwaitret;
	}
	restore(ps);
	return(OK);
}

