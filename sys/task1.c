#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#ifndef DEFAULT_LOCK_PRIO
#define DEFAULT_LOCK_PRIO 20
#endif
#ifndef assert(x,error)
#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
#endif
/*----------------------------------Tests---------------------------*/


void reader5 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
	sleep (3);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer5 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (3);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer55 (char *msg, int lck1,int lck2)
{
        kprintf ("  %s: to acquire lock l1 and l2 \n", msg);
        lock (lck2, WRITE, DEFAULT_LOCK_PRIO);
        kprintf(" %d \n %d ", lck1,lck2);
        lock (lck1, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (3);
        kprintf ("  %s: to release lock1 and lock 2 \n", msg);
        releaseall (1, lck2);
        releaseall (1,lck1);
}
void reader7(char *msg, int sem)
{
        kprintf(" %s: to acquire lock\n", msg);
        wait(sem);
        kprintf(" %s: acquired lock\n", msg);
        sleep(3);
        kprintf(" %s: to release lock\n", msg);
        signal(sem);

}


void reader77(char *msg, int sem1, int sem2)
{
        kprintf(" %s: to acquire lock 1 and 2 \n", msg);
        wait(sem2);
        wait(sem1);
        kprintf(" %s: acquired lock\n", msg);
        sleep(3);
        kprintf(" %s: to release lock\n", msg);
        signal(sem2);
        signal(sem1);

}
void writer7(char *msg, int sem)
{
        kprintf(" %s: to acquire lock\n", msg);
        wait(sem);
        kprintf(" %s: acquired lock\n", msg);
        sleep(3);
        kprintf(" %s: to release lock\n", msg);
        signal(sem);

}


void test7()
{
        int     sem1,sem2;
        int     rd1, rd2, rd3;
        int     wr1,wr2;

        kprintf("\nTest 1: test the basic priority inheritence with semaphore implementation\n");
        sem1 = screate (1);
        sem2 = screate (1);
        assert (sem1 != SYSERR, "Test 1 failed");
        assert (sem2 != SYSERR, "Test 1 failed");

        rd1 = create(reader77, 2000, 30, "reader3", 2, "reader A", sem1,sem2);
        rd2 = create(reader7, 2000, 40, "reader3", 2, "reader B", sem2);
        rd3 = create(reader7, 2000, 50, "reader3", 2, "reader C", sem1);

        kprintf("\n Start Reader A(30), then sleep 1s. 2 sems granted to Reader A ");
        resume(rd1);
        sleep (1);


        kprintf("\n Start  Reader B(40), then sleep 1s. waits for A to release sem2 " );
        resume(rd2);
        sleep (1);


        kprintf("\n Start  Reader C(50), then sleep 1s. waist for A to release sem1 ");
        resume(rd3);
        sleep (1);


        sleep(15);
}






void test5 ()
{
        int     lck1,lck2;
        int     wr1,wr2,wr3;

        kprintf("\nTest 3: Test under the Priority Inheritance Implementation\n");
        lck1  = lcreate ();
        lck2 = lcreate();
        assert (lck1 != SYSERR, "Test 3 failed");


        wr1 = create(writer55, 2000, 30, "reader3", 3, "Writer A", lck1,lck2);
        wr2 = create(writer5, 2000, 40, "reader3", 2, "Writer B", lck2);
        wr3 = create(writer5, 2000, 50, "reader3", 2, "Writer C", lck1);
        

        kprintf("-start reader A, then sleep 1s. reader A(prio 30) gets the lock\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader B, then sleep 1s. reader B(prio 40) gets the lock\n");
        resume (wr2);
	sleep (1);
	
	
        kprintf("-start reader C, then sleep 1s. reader C (prio 50) gets the lock\n");
        resume (wr3);
	sleep (1);

        sleep (10);
}
