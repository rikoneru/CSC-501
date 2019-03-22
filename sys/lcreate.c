/* lcreate.c - screate, newsem */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int ldes();

/*----------------------------------------------------------------------n
 *  * lcreate  --  create and initialize a lock, returning lock descriptor
 *   *------------------------------------------------------------------------
 *    */
SYSCALL lcreate(void)
{
        STATWORD ps;
        int     lock;

        disable(ps);
        if ( (lock=ldes())==SYSERR ) {
                restore(ps);
                return(SYSERR);
        }
        /* lqhead and lqtail were initialized at system startup */
        restore(ps);
        return(lock);
}

/*------------------------------------------------------------------------
 *  * newsem  --  allocate an unused semaphore and return its index
 *   *------------------------------------------------------------------------
 *    */
LOCAL int ldes()
{
        int     des;
        int     i;

        for (i=0 ; i<NLOCKS; i++) {
                des=nextlock--;
                if (nextlock < 0)
                        nextlock = NLOCKS-1;
                if (locktab[des].lstate==LFREE) {
                        locktab[des].lstate = LUSED;
                        return(des);
                }
        }
        return(SYSERR);
}

