#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <lock.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <stdio.h>


struct lentry locktab[NLOCKS];
int nextlock;
void linit(){
	int i;
	struct  lentry  *lptr;
	nextlock = NLOCKS-1;
	for (i=0 ; i<NLOCKS ; i++) {      /* initialize locks */
                (lptr = &locktab[i])->lstate = LFREE;
		lptr->ltype = LNONE;
		lptr->nreaders = 0;
                lptr->lqtail = 1 + (lptr->lqhead = newqueue());
	 }
}
