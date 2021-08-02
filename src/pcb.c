/* SPDX-License-Identifier: GPL-2.0 */
/**
 *  @file 
 * 
 *  @brief Implementation of the pcb.e interface. 
 *  
 *  The file implement the process queue handling and the child tree. 
 *  As constraint it must use the linux kernel linked list and the 
 *  container_of macro because the  interface does not make information 
 *  hiding about this. The use of linux kernel linked list and 
 *  container_of macro is a good idea but may be better to hide this 
 *  from interface. 
 * 
 *  @note This module may be to boost with more asbtraction and with a 
 *  skip list priority queue implementation that reside on the linux 
 *  kernel linked list. For now only FIFO is handled
 * 
 *  @see pcb.e for info about the interface 
 *  @copyright  GNU General Public License, version 2 
 * 
 */
 
 
#include "../h/listx.h"
#include "../umps/const.h"
#include "../umps/base.h"
#include "../e/pcb.e"
#include "../e/libumps.e"
#include "../umps/uMPStypes.h"


#ifdef DEBUG_OUT
#include <stdlib.h>
#endif

/**
  * @brief static process array. At most MAXPROC process can be allocated in the system
*/
HIDDEN pcb_t pcbTable[MAXPROC];

/**
  * @brief static flag fro module initialization
 */
HIDDEN int init_pcb=FALSE;

/**
  * @brief pcb_t free list
 */
HIDDEN struct list_head pcbFree_h;


/**
  * @brief macro for initPcbs() call checking
 */
#define IS_INIT_PCB     (init_pcb==TRUE)


void initPcbs();
pcb_t *allocPcb();
void freePcb(pcb_t *p);
void mkEmptyProcQ(struct list_head *emptylist);
int emptyProcQ(struct list_head *head);
pcb_t *headProcQ(struct list_head *head);
void insertProcQ(struct list_head *head, pcb_t *p);
pcb_t *outProcQ(struct list_head *head,pcb_t *p);
pcb_t *removeProcQ(struct list_head *head);
int emptyChild(pcb_t *p);
pcb_t *removeChild(pcb_t *parent);
pcb_t *outChild(pcb_t *child);
void insertChild(pcb_t *parent, pcb_t *child);
int count_actives(void);
pcb_t *resolvePid(int pid);



HIDDEN void clear_pcb(pcb_t *p);
HIDDEN int actives;
HIDDEN U32 pidtotal;
void PI(int a){}


/* clear all the pcb_t field*/
HIDDEN void clear_pcb(pcb_t *p)
{
	int i;
	
	p->p_prnt=NULL;
	p->p_semAdd=NULL;
	INIT_LIST_HEAD(&p->p_next);
	INIT_LIST_HEAD(&p->p_child);
	INIT_LIST_HEAD(&p->p_sib);
	p->start_time = 0;
	p->cpu_time = 0;
	p->pid=MAXPROC;

	for(i=0;i<3;i++)
		p->flag[i]=FALSE;
	for(i=0;i<6;i++)
		p->SPCVEC[i]=NULL;

}


void initPcbs()
{
	register int i;

	if(IS_INIT_PCB) PANIC();
	pidtotal =0;
	actives=0;
	for(i=0;i<MAXPROC;i++)
	{
		clear_pcb(&pcbTable[i]);
	}
	INIT_LIST_HEAD(&pcbFree_h);
	for(i=0;i<MAXPROC;i++)
	{

		list_add(&pcbTable[i].p_next,&pcbFree_h);
	}
	init_pcb=TRUE;	
}


pcb_t *allocPcb()
{

 	pcb_t *p=NULL;
 	U32 k=0;

	if(!IS_INIT_PCB) PANIC();
	if(list_empty(&pcbFree_h)) return NULL;
	actives++;
	p=(pcb_t*)container_of(pcbFree_h.next, struct pcb_t, p_next);
	list_del(pcbFree_h.next);
	list_del(&p->p_next);
	clear_pcb(p);
	k = (U32) p - (U32)pcbTable ;
	k= k/sizeof(pcb_t);
	p->pid= k;
	return p;
}


void freePcb(pcb_t *p)
{
	if(p==NULL) PANIC();
	actives-=1;
	clear_pcb(p);
	list_add(&p->p_next,&pcbFree_h);
}

void mkEmptyProcQ(struct list_head *emptylist)
{
	INIT_LIST_HEAD(emptylist);
}

int emptyProcQ(struct list_head *head)
{
	return list_empty(head);
}

pcb_t *headProcQ(struct list_head *head)
{
	pcb_t *p;

	if(list_empty(head) ) return NULL;

	p=(pcb_t*)container_of(head->next, struct pcb_t, p_next);
	return p;
}
void insertProcQ(struct list_head *head, pcb_t *p)
{
#ifdef DEBUG_OUT
	printf("insert proc\n");
#endif
	if(head==NULL || p==NULL) PANIC();

	list_add_tail(&p->p_next,head);
}

pcb_t *outProcQ(struct list_head *head,pcb_t *p)
{
	pcb_t *ptr=NULL;
	struct list_head *t=NULL;


	if(emptyProcQ(head)) return NULL;

	list_for_each(t,head)
	{
		ptr=container_of(t,pcb_t,p_next);
		if(ptr==p)
		{
			list_del(&ptr->p_next);
			return ptr;
		}
	}
	return NULL;
}

pcb_t *removeProcQ(struct list_head *head)
{
	pcb_t *p;

	if(emptyProcQ(head) ) return NULL;

	p=container_of(head->next,pcb_t,p_next);
	list_del(&p->p_next);
	 mkEmptyProcQ(&p->p_next);
	return p;

}


pcb_t *resolvePid(int pid)
{
	return &pcbTable[pid];
}

int count_actives()
{
	return actives;
}


int emptyChild(pcb_t *p)
{
	return list_empty(&p->p_child);
}


pcb_t *removeChild(pcb_t *parent)
{
	pcb_t * new = NULL;
	struct list_head *q = &parent->p_child;


	if(list_empty(&parent->p_child)) return NULL;
	new = (pcb_t*) container_of(q->next,pcb_t,p_sib);
	list_del(&new->p_sib);

	new->p_prnt=NULL;
	return new;
}



pcb_t *outChild(pcb_t *child)
{

	if(child->p_prnt == NULL) return NULL;
	child->p_prnt=NULL;
	list_del(&child->p_sib);
	return child;
}



void insertChild(pcb_t *parent, pcb_t *child)
{
	outChild(child);
	child->p_prnt = parent;
	list_add(&child->p_sib,&parent->p_child);
}

