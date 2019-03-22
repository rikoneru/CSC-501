#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCKS
#define NLOCKS 50  /* if number of locks not defined */
#endif

#define LNONE '\00'  /*This Lock is not associated with type for initialization purpose */
#define LFREE '\01'  /* This Lock is free */
#define LUSED '\02'  /* This Lock is Used */
#define READ '\03'  /* This lock is Read */
#define WRITE '\04' /* This lock is Write */

struct lentry {            /* Lock Table Entry */
	char lstate;       /* State of Lock */
	char ltype;	   /* Type of Lock */
	int lqhead;	   /* q index of head of list */
        int lqtail;        /* q index of tail of list */
	int nreaders;      /* Number of Readers */
	int hprocs[50];    /* Holding Processes */
	int dprocs[50];	   /* For Lock Deletion Handling */
	int lprio;			/* Highest Priority waiting for lock */
};

extern  struct  lentry  locktab[];
extern  int     nextlock;
#define isbadlock(l)     (l<0 || l>=NLOCKS)
#endif
void linit();
int  lcreate();
int ldelete(int);
int lock(int,int,int);
int releaseall(int,int,...);
