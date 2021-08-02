/* SPDX-License-Identifier: GPL-2.0 */
/**
 *  @brief The scheduler Implememtation
 *  
 *  The file implement the scheduler interface defined in scheduler.e 
 *  and it integrate it with a round robin FIFO scheduler. 
 *  Most of the function defined here are hidden to the caller.
 *  
 *  @note This source code should be extended with a Priority Queue for 
 *  mulitple scheduling strategy
 *  @see scheduler.e for info about the interface
 *  @copyright  GNU General Public License, version 2 

 *  @file
 *
 **/

#include "../h/listx.h"
#include "../h/types10.h"
#include "../h/scheduler.h"

#include "../e/scheduler.e"
#include "../e/asl.e"
#include "../e/libumps.e"
#include "../e/pcb.e"

#include "../umps/base.h"
#include "../umps/const.h"
#include "../umps/uMPStypes.h"



HIDDEN state_t idleState;
HIDDEN int initialized = FALSE;

/**
  * @brief Ready Queue
  */
struct list_head readyQ;

/**
  * @brief idle pcb_t
  */
pcb_t waiting;

void state_t_copy(state_t* src, state_t* dst);
void extern_scheduler(int flag, int interruptFlag);
void time(int now);


/**
  * @brief The main scheduler procedure. If Ready Queue is empty run the 
  * idle process. Otherwise run the next process in FIFO order 
  * @param: void
  * @return void
  * @note When count_actives() and BlockCount() are 0 the system HALT() 
  * because there is no process to execute.
  * Aslo when the Ready Queue is empty but count_actives()>0 and there 
  * are no blocked prccess the system is in deadlock and the PANIC() function 
  * is raised.
  */
HIDDEN void scheduler(void);

/**
  * @brief This function directly load the old area after an interrupt. 
  * It is to call in the interrupt handler to avoid scheduler context switch
  * @param void;
  * @return void
  */
HIDDEN void intsched(void);

/**
  * @brief This function directly load the old area after a SYSCALL calling. 
  * It is to call after the system call code in the handler to avoid scheduler context switch
  * @param void;
  * @return void
  */
HIDDEN void syssched(void);

/**
  * @brief Make a context switch in the scheduler
  * @param int when 1 is passed the function take the time stat for 
  * the current process and  call the scheduler(), otherwise the 
  * function call only scheduler()
  * @return void
  */
HIDDEN void contextswitch(int flag);

/**
  * @brief Set the timer interrupt for the current process to compute 
  * the remaining time slice. 
  * @param void;
  * @return void
  */
HIDDEN void set_alarm(void);

/**
  * @brief Set the timer in agree to pseudoclock
  * @param void;
  * @return void
  */
HIDDEN void set_alarm2pseudoclock(void);

/**
  * @brief Set the next pseudoclock and run the idle procedure
  * @param void;
  * @return void
  */
HIDDEN void idlegoing();

/**
  * @brief Function for the idle process
  * @param void;
  * @return void
  */
HIDDEN void idle()
	{
	while (emptyProcQ(&readyQ));;
		scheduler();
	}
	

void extern_scheduler(int flag, int interruptFlag)
	{
		switch (flag)
		{
			case CONTEXTSWITCH:
				contextswitch(interruptFlag);
				break;
			case SYSSCHED:
			syssched();
				break;
			case INTSCHED:
				intsched();
		}
	}


HIDDEN void intsched()
{
	if(running != &waiting)
		{
		set_alarm();
		LDST((state_t*) INT_OLDAREA);
		}
	else
		{
		scheduler();
		}
}


HIDDEN void syssched()
{
	time(GET_TODLOW);
	set_alarm();
	LDST((state_t*) SYSBK_OLDAREA);
}


HIDDEN void contextswitch(int flag)
{
	if(!flag) time(GET_TODLOW);

	scheduler();
}


HIDDEN void scheduler() {

	if (emptyProcQ(&readyQ) )
	{
		int k = count_actives();
		int q = BlockCount();
		
		/*no process in the system*/
		if (k == 0 && q == 0) HALT();
		
		/* deadlock */
		if (k> 0 && q== 0) PANIC();
		
		/* wait state */
		if (k > 0 && q > 0) idlegoing();

	}
	else
	{

		running = removeProcQ(&readyQ);
		running->start_time = GET_TODLOW;
		running->remaining = SCHED_TIME_SLICE;
		set_alarm();
		LDST(&running->p_state);
	}
}


void state_t_copy(state_t* src, state_t* dst) 
{
	int i;
	dst->lo = src->lo;
	dst->cause = src->cause;
	dst->status = src->status;
	dst->pc_epc = src->pc_epc;
	for (i = 0; i < STATEREGNUM; i++) 
		dst->gpr[i] = src->gpr[i];
	
}


void init_Sched(pcb_t *first_proc)
{
	if (initialized == FALSE)
	{
		running = NULL;
		mkEmptyProcQ(&readyQ);
		insertProcQ(&readyQ, first_proc);
		initialized = TRUE;
		STST(&idleState);
		idleState.pc_epc = idleState.reg_t9 = (memaddr) idle;
		idleState.reg_sp -= 2048;

		idleState.status = 0;
		idleState.cause = 0;
		idleState.status |= 0xFC00 | 0x4;
		idleState.reg_t9 = idleState.pc_epc = (memaddr) idle;
		state_t_copy((state_t*) &idleState,(state_t*) &(waiting.p_state));

		SET_IT(SCHED_TIME_SLICE);
		scheduler();
	}
	else
	{
		PANIC();
	}


}

HIDDEN void idlegoing()
{
	running=&waiting;
	set_alarm2pseudoclock();
	LDST(&waiting.p_state);
}

void time(int now)
{
	if(running != &waiting)
		{
		running->cpu_time += now - running->start_time;
		running->remaining -= now - running->start_time;
		running->start_time = now;
		}
}


void set_alarm()
{
	if(running->remaining >0)
		{
			int now = GET_TODLOW;
			if(pseudoclock< now + running->remaining)
			{
				if(pseudoclock <= now) SET_IT(0);
				else SET_IT( pseudoclock - now );

			}
			else SET_IT(running->remaining);
		}

}


void set_alarm2pseudoclock()
{

	int now = GET_TODLOW;
	if(pseudoclock<= now) SET_IT(0);
	else SET_IT(pseudoclock - now);
}
