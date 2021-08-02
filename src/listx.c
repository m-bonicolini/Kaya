/* (C) 2009 Renzo Davoli
 * Licensed under the GPLv2
 */

#include <stdio.h>
#include <stdlib.h>
#include "pcb.c"

/*
struct list_head testhead=LIST_HEAD_INIT(testhead);

struct listofint {
	int value;
	struct list_head list;
};

char buf[256];

void listlist(void)
void listlist(void)
{
	struct listofint *pos;
	list_for_each_entry(pos, &testhead, list)
		printf("%d\n",pos->value);
	printf("===\n");
}

main()
{
	while (gets(buf) != NULL) {
		struct listofint *new=malloc(sizeof(struct listofint));
		new->value=atoi(buf);
		list_add_tail(&new->list, &testhead);
		listlist();
	}
}
*/
int main() {
	int i;
//static int MAXSEM=20;

char okbuf[2048];			/* sequence of progress messages */
char errbuf[128];			/* contains reason for failing */
char msgbuf[128];			/* nonrecoverable error message before shut down */
int onesem;
int sem[MAXSEM];
pcb_t	*procp[20], *p, *q, *firstproc, *lastproc, *midproc;
struct list_head qa;

	initPcbs();
	printf("Initialized process control blocks   \n");
	printf("prova bonko %d\n",1<<0);
	/* Check allocPcb */
	for (i = 0; i < MAXPROC; i++) {
		if ((procp[i] = allocPcb()) == NULL)
			printf("allocPcb(): unexpected NULL   ");
	}
	if (allocPcb() != NULL) {
		printf("allocPcb(): allocated more than MAXPROC entries   ");
	}
	printf("allocPcb ok   \n");

	/* return the last 10 entries back to free list */
	for (i = 10; i < MAXPROC; i++)
		freePcb(procp[i]);
	printf("freed 10 entries   \n");

	/* create a 10-element process queue */
	INIT_LIST_HEAD(&qa);
	if (!emptyProcQ(&qa)) printf("emptyProcQ(qa): unexpected FALSE   ");
	printf("Inserting...   \n");
	for (i = 0; i < 10; i++) {
		if ((q = allocPcb()) == NULL)
			printf("allocPcb(): unexpected NULL while insert   ");
		switch (i) {
		case 0:
			firstproc = q;
			break;
		case 5:
			midproc = q;
			break;
		case 9:
			lastproc = q;
			break;
		default:
			break;
		}
		insertProcQ(&qa, q);
	}
	printf("inserted 10 elements   \n");

	if (emptyProcQ(&qa)) printf("emptyProcQ(qa): unexpected TRUE"   );

	/* Check outProcQ and headProcQ */
	if (headProcQ(&qa) != firstproc)
		printf("headProcQ(qa) failed   ");
	printf("primo head proc ok\n");
		
	q = outProcQ(&qa, firstproc);
	if ((q == NULL) || (q != firstproc))
		printf("outProcQ(&qa, firstproc) failed on first entry   ");		
	freePcb(q);
	
	q = outProcQ(&qa, midproc);
	if (q == NULL || q != midproc)

		printf("outProcQ(&qa, midproc) failed on middle entry   ");
	freePcb(q);
	
	//printf("prima di entry inesistente\n");
	pcb_t * k =outProcQ(&qa, procp[0]);
	if (k != NULL)
		if(procp[0]->pid == k->pid)
		{
			printf("outProcQ(&qa, procp[0]) failed on nonexistent entry   ");
			printf("pid1=%u pid2=%u \n ",k->pid,procp[0]->pid);
			fflush(stdout);
		}
			printf("outProcQ() ok   \n");

	/* Check if removeProc and insertProc remove in the correct order */
	printf("Removing...   \n");
	for (i = 0; i < 8; i++) {
	printf("i=%d \n",i);
		if ((q = removeProcQ(&qa)) == NULL)
			printf("removeProcQ(&qa): unexpected NULL   ");
		freePcb(q);
	}
	
	if (q != lastproc)
		printf("removeProcQ(): failed on last entry   \n");
		
	if (removeProcQ(&qa) != NULL)
		printf("removeProcQ(&qa): removes too many entries   \n");

  if (!emptyProcQ(&qa))
    printf("emptyProcQ(qa): unexpected FALSE   \n");

	printf("insertProcQ(), removeProcQ() and emptyProcQ() ok   \n");
	printf("process queues module ok      \n");
	

	printf("Controlliamo gli alberi   \n");
	printf("checking process trees...\n");

	if (!emptyChild(procp[2]))
	  printf("emptyChild: unexpected FALSE   ");

	/* make procp[1] through procp[9] children of procp[0] */
	printf("Inserting...   \n");
	for (i = 1; i < 10; i++) {
		insertChild(procp[0], procp[i]);
	}
	printf("Inserted 9 children   \n");

	if (emptyChild(procp[0]))
	  printf("emptyChild(procp[0]): unexpected TRUE   ");

	/* Check outChild */
	q = outChild(procp[1]);
	if (q == NULL || q != procp[1])
		printf("outChild(procp[1]) failed on first child   ");
	q = outChild(procp[4]);
	if (q == NULL || q != procp[4])
		printf("outChild(procp[4]) failed on middle child   ");
	if (outChild(procp[0]) != NULL)
		printf("outChild(procp[0]) failed on nonexistent child   ");
	printf("outChild ok   \n");

	/* Check removeChild */
	printf("Removing...   \n");
	for (i = 0; i < 7; i++) {
		if ((q = removeChild(procp[0])) == NULL)
			printf("removeChild(procp[0]): unexpected NULL   ");
	}

	if (removeChild(procp[0]) != NULL)
	  printf("removeChild(): removes too many children   ");

	if (!emptyChild(procp[0]))
	    printf("emptyChild(procp[0]): unexpected FALSE   ");

	printf("insertChild(), removeChild() and emptyChild() ok   \n");
	printf("process tree module ok      \n");

	for (i = 0; i < 10; i++)
		freePcb(procp[i]);

	/* check ASL */
	initSemd();
	printf("Initialized active semaphore list.TEST SEMAFORI START   \n");

	/* check removeBlocked and insertBlocked */
	printf("insertBlocked() test #1 started  \n");
	for (i = 10; i < MAXPROC; i++) {
		procp[i] = allocPcb();
		if (insertBlocked(&sem[i], procp[i]))
			printf("insertBlocked() test#1: unexpected TRUE   ");
	}
	printf("insertBlocked() test #2 started  \n");
	for (i = 0; i < 10; i++) {
		procp[i] = allocPcb();
		if (insertBlocked(&sem[i], procp[i]))
			printf("insertBlocked() test #2: unexpected TRUE   ");
	}
	printf("mi accingo a fare LA REMOVE\n");
	fflush(stdout);
	/* check if semaphore descriptors are returned to free list */
	p = removeBlocked(&sem[11]);
	if (insertBlocked(&sem[11],p))
		printf("removeBlocked(): fails to return to free list   ");

	if (insertBlocked(&onesem, procp[9]) == FALSE)
		printf("insertBlocked(): inserted more than MAXPROC   ");
	
	printf("removeBlocked() test started   \n");
	for (i = 10; i< MAXPROC; i++) {
		q = removeBlocked(&sem[i]);
		if (q == NULL)
			printf("removeBlocked(): wouldn't remove   ");
		if (q != procp[i])
			printf("removeBlocked(): removed wrong element   ");
		if (insertBlocked(&sem[i-10], q))
			printf("insertBlocked(3): unexpected TRUE   ");
	}
	if (removeBlocked(&sem[11]) != NULL)
		printf("removeBlocked(): removed nonexistent blocked proc   ");
	printf("insertBlocked() and removeBlocked() ok   \n");

	if (headBlocked(&sem[11]) != NULL)
		printf("headBlocked(): nonNULL for a nonexistent queue   ");
	if ((q = headBlocked(&sem[9])) == NULL)
		printf("headBlocked(1): NULL for an existent queue   ");
	if (q != procp[9])
		printf("headBlocked(1): wrong process returned   ");
	p = outBlocked(q);
	if (p != q)
		printf("outBlocked(1): couldn't remove from valid queue   ");
	q = headBlocked(&sem[9]);
	if (q == NULL)
		printf("headBlocked(2): NULL for an existent queue   ");
	if (q != procp[19])
		printf("headBlocked(2): wrong process returned   ");
	p = outBlocked(q);
	if (p != q)
		printf("outBlocked(2): couldn't remove from valid queue   ");
	p = outBlocked(q);
	if (p != NULL)
		printf("outBlocked(): removed same process twice.");
	if (headBlocked(&sem[9]) != NULL)
		printf("out/headBlocked: unexpected nonempty queue   ");
	printf("headBlocked() and outBlocked() ok   \n");
	printf("ASL module ok   \n");
	printf("So Long and Thanks for All the Fish\n");
	 return 0;
	}
