/* SPDX-License-Identifier: GPL-2.0 */
/**
 *  @brief Implementation of the Active Semaphore List. 
 *  
 *  The file define low level semaphore handling algorithm. It 
 *  implement data struct for fair sempahore with an 
 *  allocate() / free() logic. The semaphore data struct is a 32 bit 
 *  integer for the caller but internally it is a semd_t handled with 
 *  the free list algorithm
 *  
 * 
 *  @see asl.e for info about the interface
 *  @copyright GNU General Public License, version 2 
 *  @file
 *
 **/
#include "../h/listx.h"
#include "../umps/const.h"
#include "../umps/base.h"
#include "../e/pcb.e"
#include "../e/libumps.e"
#include "../e/asl.e"


/**
  * @brief Max number of allocable seamphore 
  */
#define MAXSEM MAXPROC 

/**
  * @brief static semaphore vector
  */
HIDDEN semd_t semTable[MAXSEM]; 

HIDDEN int init_sem=FALSE;
HIDDEN struct list_head s_head;
HIDDEN struct list_head *s_list;
HIDDEN struct list_head semdFree_h;
HIDDEN struct list_head *f_sem;
HIDDEN int blocked;




int insertBlocked(S32 *semAdd, pcb_t *p);
pcb_t *outBlocked(pcb_t *p);
pcb_t *removeBlocked(S32 *semAdd);
pcb_t *headBlocked(S32 *semAdd);
void initSemd(void);
int BlockCount(void);

/**
*  @brief Dequeue a semd_t from a free list FIFO
*  @param void
*  @return a pointer to a semd_t if it can be allocated,NULL otherwise
*/
HIDDEN semd_t* AllocSem(void);

/**
*  @brief Insert a semd_t in the hidden free list FIFO
*  @param semd_t* the semaphore to deallocate
*  @return void
*/
HIDDEN void freeSem(semd_t *sem);
HIDDEN semd_t* seek_sem(S32 *semAdd);



HIDDEN semd_t* AllocSem()
{

 	semd_t *p=NULL;

	if(init_sem==FALSE)	PANIC();

	if(list_empty(f_sem)) return NULL;

	p=(semd_t*)container_of(f_sem->next, semd_t, s_next);

	list_del(f_sem->next);
	list_del(&p->s_next);

	return p;

}

HIDDEN void freeSem(semd_t *sem)
{

	list_add(&sem->s_next,f_sem);
}

HIDDEN semd_t* seek_sem(S32 *semAdd)
{
	semd_t *ptr=NULL;
	struct list_head *t=NULL;


	if(list_empty(s_list)) return NULL;

	list_for_each(t,s_list)
	{
		ptr=(semd_t*)container_of(t, struct semd_t, s_next);
		if(ptr->s_semAdd==semAdd) return ptr;
	}

	return NULL;
}

int insertBlocked(S32 *semAdd, pcb_t *p)
{
	semd_t *ptr=NULL;
	struct list_head *t=NULL;
	semd_t *pt=NULL;

	if(init_sem==FALSE)PANIC();

	if(p==NULL || semAdd == NULL) return TRUE;

	blocked+=1;
	ptr=seek_sem(semAdd);
	if(ptr!=NULL)	
	{
		insertProcQ(&ptr->s_procQ,p);
		p->p_semAdd = semAdd;
		return FALSE;
	}
	ptr=AllocSem();
	if(ptr==NULL) return TRUE;
	insertProcQ(&ptr->s_procQ,p);
	p->p_semAdd = semAdd;
	ptr->s_semAdd=semAdd;
	
	
	list_for_each(t,s_list)
	{
		pt=(semd_t*)container_of(t, struct semd_t, s_next);
		if(ptr->s_semAdd < pt->s_semAdd) break;
	}
	
	list_add(&ptr->s_next,t);	
	return FALSE;
}

pcb_t *outBlocked(pcb_t *p)
{
	semd_t *ptr=NULL;
	pcb_t *ret=NULL;
	
	if(init_sem==FALSE)PANIC();
	
	if(p==NULL) return NULL;
	ptr=seek_sem(p->p_semAdd);
	if(ptr == NULL) return NULL;

	if(headBlocked(p->p_semAdd)== p)
	{
		return removeBlocked(p->p_semAdd);
	}

	ret=outProcQ(&ptr->s_procQ,p);

	if(ret== NULL) return NULL;

	ret->p_semAdd = NULL;
	blocked -=1;
	return ret;
}


pcb_t *removeBlocked(S32 *semAdd)
{
	semd_t *ptr=NULL;
	pcb_t *ret=NULL;

	if(init_sem==FALSE)PANIC();
	
	if(semAdd == NULL) return NULL;
	ptr=seek_sem(semAdd);
	if(ptr==NULL) return NULL;

	ret=removeProcQ(&ptr->s_procQ);
	if(list_empty(&ptr->s_procQ))	
	{
		ptr->s_semAdd=NULL;
		list_del(&ptr->s_next);
		freeSem(ptr);
	}

	ret->p_semAdd = NULL;
	blocked -=1;
	return ret;
}


pcb_t *headBlocked(S32 *semAdd)
{
	semd_t *p=NULL;

	if(init_sem==FALSE)PANIC();
	
	if(semAdd == NULL) return NULL;
	p=seek_sem(semAdd);
	if(p==NULL) return NULL;

	return headProcQ(&p->s_procQ);
}



void initSemd(void)
{
	int i;

	INIT_LIST_HEAD(&s_head);
	INIT_LIST_HEAD(&semdFree_h);
	for (i = 0; i < MAXSEM; i++)
	{
		INIT_LIST_HEAD(&semTable[i].s_next);
		INIT_LIST_HEAD(&semTable[i].s_procQ);
		semTable[i].s_semAdd=NULL;
		semTable[i].indice=(U8)i;
		list_add(&(semTable[i].s_next),&semdFree_h);
	}
	s_list=&s_head;
	f_sem=&semdFree_h;
	init_sem=TRUE;
	blocked = 0;
}

int BlockCount()
{
	if(init_sem==FALSE)PANIC();
	
	return blocked;
}

struct list_head *sem_list()
 {
    return s_list;
 }
