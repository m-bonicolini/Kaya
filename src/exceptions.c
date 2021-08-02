/* SPDX-License-Identifier: GPL-2.0 */
/**
 *  @brief Implementation of the KAYA handlers
 *  
 *  This file is the source code of the System Call handler, 
 *  TLB and PGMTRAP Handler. Also it implement
 *  the code of the KAYA System Call. The only public function is 
 *  defined in the exceptions.e interface and it is the init_exceptions().
 *
 * 
 *  @see exceptions.e for info about the implementation 
 *  @copyright  GNU General Public License, version 2 
 *  @file
 *
 **/
#include "../h/listx.h"
#include "../h/scheduler.h"
#include "../h/types10.h"
#include "../umps/uMPStypes.h"
#include "../umps/const.h"
#include "../umps/base.h"
#include "../e/scheduler.e"
#include "../e/exceptions.e"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "/usr/include/uMPS/libumps.e"


/**
  * @brief macro to check kernel mode
  */
#define KUPBITON				0x8

/**
  * @brief reg_a0 of the sysbk_oldarea.
  */
#define PARAMETER0 ( ((state_t * ) SYSBK_OLDAREA)->reg_a0)
/**
 * @brief reg_a1 of the sysbk_oldarea.
 */
#define PARAMETER1 ( ((state_t * ) SYSBK_OLDAREA)->reg_a1)
/**
 * @brief reg_a2 of the sysbk_oldarea.
 */
#define PARAMETER2 ( ((state_t * ) SYSBK_OLDAREA)->reg_a2)

/**
 * @brief reg_a3 of the sysbk_oldarea.
 */
#define PARAMETER3 ( ((state_t * ) SYSBK_OLDAREA)->reg_a3)
/**
 * @brief reg_v0 of the sysbk_oldarea.
 */
#define PARAMETER4 ( ((state_t * ) SYSBK_OLDAREA)->reg_v0)


/**
 * @brief macro for the  "Burn the heretics" to kill running process
 */
#define RUNNING -1


/*macro per la gestione del vettore eccezioni*/
/**
 * @brief macro for the TLBTRAP
 */
#define TLB 0

/**
 * @brief macro for the PGMTRAP
 */
#define PGM 1


/**
 * @brief macro for the SYSCALL
 */
#define SYS 2


/**
 * @brief macro for custom TLB
 */
#define TLBSPEC 0
/**
 * @brief macro for custom PGM
 */
#define PGMSPEC 2
/**
 * @brief macro for customSYSCALL
 */
#define SYSSPEC 4

#define IS_INIT_HANDLER  (i_hand == TRUE);

extern int dev_sem[4][8];
extern int dev_term[2][8];
extern int dev_status[4][8];
extern int dev_status_term[2][8];
extern struct list_head readyQ;
extern pcb_t* running;

unsigned int is_kernel_mode(unsigned int status);

HIDDEN unsigned int i_hand = FALSE;
HIDDEN state_t *PgmTrap=(state_t *)PGMTRAP_NEWAREA;
HIDDEN state_t *Bk=(state_t *)SYSBK_NEWAREA;
HIDDEN state_t *Tlb=(state_t *)TLB_NEWAREA;


/**
  * @brief TLB handler
  * @param void;
  * @return void
  */
HIDDEN void tlb_handler(void);


/**
  * @brief SYSCALL handler
  * @param void;
  * @return void
  */
HIDDEN void sys_bp_handler(void);

/**
  * @brief PGMTRAP handler
  * @param void;
  * @return void
  */
HIDDEN void pgm_handler(void);


/**
  * @brief Implement the create process SYSCALL by creating a son process
  * @param state_t parent processor state 
  * @return int the pid of the created process
  */
HIDDEN int create_process(state_t *p);

/**
  * @brief Kill a process by pid and its progenie
  * @param int process pid
  * @return -1 if the function fail, otherwise 0 
  */
HIDDEN int burn_the_heretics(int pid);

/**
  * @brief Kill the running process and it make extern_scheduler(CONTEXTSWITCH,FALSE)
  * @param void;
  * @return void;
  */
HIDDEN void killRunning(void);

/**
  * @brief Make a P() on a sempahore. 
  * If it block make extern_scheduler(CONTEXTSWITCH,FALSE)
  * @param int*  a semaphore pointer 
  * @return void;
  */
HIDDEN void passern(int *sem);

/**
  * @brief Check if semaphore is a system semaphore.
  * @param int* the semaphore to check
  * @return 1 the semAdd is found, 0 otherwise
  */
HIDDEN int findSem(int* semAdd);

/**
  * @brief Make V() on a semaphore. The call is non-blocking
  * @param int* a semaphore pointer 
  * @return pcb_t* the unblocked process
  */
pcb_t* verhogen(int *sem);

/**
  * @brief SPECVEC TLB, PGM  and SYS handling
  * @param int set the handler to run
  * @return void
  */
HIDDEN void spec_vect(int index);

/**
  * @brief SYSCALL exception handler
  * @param void;
  * @return void;
  */
HIDDEN void syscallBK(void);


/**
  * @brief Check if the calling process has the privilege to call any SYSCALL
  * @param void;
  * @return void;
  */
HIDDEN int is_a_legal_request(void);




void init_exceptions()
{

	if(i_hand==TRUE)PANIC();
	
	i_hand=TRUE;

	Bk-> pc_epc = Bk->reg_t9 = (memaddr)sys_bp_handler;
	Bk-> reg_sp = (memaddr)RAMTOP-(5*1024);
	Bk-> status = (unsigned int)0x10000000;

	Tlb-> pc_epc = Tlb->reg_t9 = (memaddr)tlb_handler;
	Tlb-> reg_sp = (memaddr)RAMTOP-(6*1024);
	Tlb-> status = (unsigned int)0x10000000;

	PgmTrap-> pc_epc = PgmTrap->reg_t9 = (memaddr) pgm_handler;
	PgmTrap-> reg_sp = (memaddr)RAMTOP-(7*1024);
	PgmTrap->status = (unsigned int)0x10000000;
}


HIDDEN void sys_bp_handler()
{
	time(GET_TODLOW);

	if((!is_kernel_mode(((state_t*)SYSBK_OLDAREA)->status) && is_a_legal_request()))
	{
		state_t_copy((state_t *) SYSBK_OLDAREA,(state_t *) PGMTRAP_OLDAREA);
		((state_t*) PGMTRAP_OLDAREA)->cause |= 0x00000028; 
		pgm_handler();
	}
	else
	{
		if(!is_a_legal_request()) syscallBK();
		
		((state_t*)SYSBK_OLDAREA)->reg_t9 = (((state_t*)SYSBK_OLDAREA)->pc_epc+=WORD_SIZE);
		switch(PARAMETER0)
		{
			case CREATEPROCESS:
			{
				PARAMETER4=create_process((state_t *)((state_t*)SYSBK_OLDAREA)->reg_a1);
			}break;
			case TERMINATEPROCESS:
			{
			if(PARAMETER1 == RUNNING)
				{
				killRunning();
				}
				else
				{
				PARAMETER4 = burn_the_heretics((int)((state_t*)SYSBK_OLDAREA)->reg_a1);
				}

			}break;
			case VERHOGEN:
			{
			verhogen((int*)PARAMETER1);
			}break;
			case PASSEREN:
			{
			passern((int*)PARAMETER1);
			}break;
			case GETPID:
			{
			PARAMETER4 = running->pid;
			}
			case GETCPUTIME:
			{
			time(GET_TODLOW);
			PARAMETER4= (U32)(running->cpu_time);
			}break;
			case WAITCLOCK:
			{
			passern(&wait4pseudoclock);
			}break;
			case WAITIO:
			{

				unsigned int int_line=0;
				unsigned int dev_num=0;
				unsigned int flag=0;
				int_line=PARAMETER1;
				dev_num=PARAMETER2;
				flag=PARAMETER3;
				if(int_line==7)
					if(dev_status_term[flag][dev_num]!= 0)
					{
						PARAMETER4=dev_status_term[flag][dev_num];
						dev_status_term[flag][dev_num]=0;
					}
					else
						passern(&dev_term[flag][dev_num]);
				else
					if(dev_status[int_line][dev_num]!= 0)
					{
						PARAMETER4=dev_status[int_line][dev_num];
						dev_status[int_line][dev_num]=0;
					}
					else
						passern(&dev_sem[int_line][dev_num]);
			}break;
			case SPECTLBVECT:
			{
			spec_vect(TLBSPEC);

			}break;
			case SPECPGMVECT:
			{
			spec_vect(PGMSPEC);
			}break;
			case SPECSYSVECT:
			{
			spec_vect(SYSSPEC);
			}break;



		}
		extern_scheduler(SYSSCHED,FALSE);

	}
}


HIDDEN void tlb_handler()
{
	time(GET_TODLOW);
	if (running->flag[TLB] == FALSE)
	{
		killRunning();
	}
	else
	{
		state_t_copy((state_t*) TLB_OLDAREA,running->SPCVEC[TLBSPEC]);
		LDST((state_t*) running->SPCVEC[TLBSPEC+1]);
	}
	extern_scheduler(CONTEXTSWITCH,FALSE);
}


HIDDEN void pgm_handler()
{
	time(GET_TODLOW);
	if (running->flag[PGM] == FALSE)
	{
		killRunning();
	}
	else
	{
		state_t_copy((state_t*) PGMTRAP_OLDAREA, running->SPCVEC[PGMSPEC]);
		LDST((state_t*) running->SPCVEC[PGMSPEC+1]);
	}
	extern_scheduler(CONTEXTSWITCH,FALSE);
}



HIDDEN int create_process(state_t *p)
{

	state_t *old_area=(state_t*) SYSBK_OLDAREA;
	state_t * proc_state=NULL;
	pcb_t *son=NULL;

	son=allocPcb();
	if(son==NULL)
	{
		old_area->reg_v0=-1;
		proc_state=&running->p_state;
		return -1;
	}
    else
	{
		proc_state=&running->p_state;
		if(((state_t*)old_area->reg_a1)==NULL) PANIC();

		state_t_copy(p,&son->p_state);
		insertChild(running,son);
		insertProcQ(&readyQ,son);

		return son->pid;

	}
}


int burn_the_heretics(int pid)
{

    struct list_head lista;
    mkEmptyProcQ(&lista);
    pcb_t* target;
    pcb_t* son;
    pcb_t* father;
    pcb_t* listafirst;

    if(pid==RUNNING)
    {
        target=running;
    }
    else
    {
        target = resolvePid(pid);
        father = running->p_prnt;
        while(father != NULL && father != target)
        {
            father = father->p_prnt;
        }
        if(father != NULL)
        {
            return -1;
        }
    }
    if(target->p_prnt != NULL)
        {
            if(outChild(target)== NULL) PANIC();

        }
    outProcQ(&readyQ,target);
    insertProcQ(&lista,target);
    while(!emptyProcQ(&lista))
    {
    listafirst=headProcQ(&lista);
    if(listafirst->p_semAdd != NULL)
        {
            if(!findSem(listafirst->p_semAdd))
                (*(int*)listafirst->p_semAdd)+=1;

            if(outBlocked(listafirst) == NULL)
                PANIC();


        }
    while(!emptyChild(listafirst))
        {
        son = removeChild(listafirst);
        outProcQ(&readyQ,son);
        if(son->p_semAdd != NULL)
            {
                if(!findSem(son->p_semAdd))
                    (*(int*)son->p_semAdd)+=1;

                if(outBlocked(son) == NULL)
                    PANIC();


            }
        insertProcQ(&lista,son);
        }
    removeProcQ(&lista);
    freePcb(listafirst);

    }
    return TRUE;

}


pcb_t* verhogen(int* sem)
{
	pcb_t *unblocked_proc=NULL;

	*sem += 1;

	if (*sem <= 0)
	{
		unblocked_proc =  removeBlocked(sem);
		if(unblocked_proc==NULL) PANIC();

		insertProcQ(&readyQ,unblocked_proc);
		return unblocked_proc;
	}
	return NULL;
}


HIDDEN void passern(int *sem)
{

	if(sem==NULL)
	{
		PANIC();
	}
	*sem -= 1;

	if (*sem < 0)
	{
		state_t_copy((state_t*)SYSBK_OLDAREA, (state_t*)&(running->p_state));
		time(GET_TODLOW);
		running->remaining = SCHED_TIME_SLICE;
		insertBlocked(sem, (pcb_t *) running);
		extern_scheduler(CONTEXTSWITCH,FALSE);
	}
}


HIDDEN int findSem(int* semAdd)
{
	register int i;
	register int j;
	
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 8; ++j)
		{
			if(semAdd == &dev_sem[i][j])
			{
				return TRUE;
			}
		}
	}
	for (i = 0; i < 2; ++i)
	{
		for (j = 0; j < 8; j++)
		{
			if(semAdd == &dev_term[i][j])
			return TRUE;
		}
	}
	return FALSE;

}

unsigned int is_kernel_mode(unsigned int status)
{
	unsigned int result = TRUE;

	if (status & KUPBITON)
		result = FALSE;

	return result;
}


HIDDEN void killRunning()
{
	burn_the_heretics(RUNNING);
	extern_scheduler(CONTEXTSWITCH,FALSE);
}


HIDDEN void spec_vect(int index)
{
	int flag;
	switch (index)
	{
		case TLBSPEC:
			flag = TLB;
		break;
		case PGMSPEC:
			flag=PGM;
		break;
		case SYSSPEC:
			flag=SYS;
		break;
	}
	if (running->flag[flag] == TRUE)
	{
		killRunning();
	}
	else
	{
		running->flag[flag] = TRUE;
		running->SPCVEC[index] = (state_t*)PARAMETER1;
		running->SPCVEC[index+1] = (state_t*)PARAMETER2;
	}
}


HIDDEN void syscallBK()
{
	if(running->flag[SYS] == TRUE)
		{
		state_t_copy((state_t*) SYSBK_OLDAREA,running->SPCVEC[SYSSPEC] );
		LDST((state_t*)running->SPCVEC[SYSSPEC+1]);
		}
	else{
		killRunning();
		}
}


HIDDEN int is_a_legal_request()
{
	if(PARAMETER0>=CREATEPROCESS && PARAMETER0<=SYSCALL_MAX)
		return TRUE;
	return FALSE;
}
