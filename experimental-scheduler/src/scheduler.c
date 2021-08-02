#include "h/types.h"
#include "h/const.h"
#include "h/kutil.h"
#include "e/libumps.e"
#include "e/threadq.e"
#include "e/asl.e"
#include "h/scheduler.h"

HIDDEN pcb_t *Current_proc=NULL; /*current proc*/
HIDDEN pcb_t *swap_queue=NULL; /*swap queue for the scheduler*/
HIDDEN pcb_t *ready_queue=NULL;
HIDDEN unsigned int round=0; /*round counteer for the current proc*/
HIDDEN state_t wait; /*custom state for wait state*/
HIDDEN unsigned int procs_count, softblock_count=0; /*process counter*/
HIDDEN unsigned int activate_sched=0; /*check module initialization*/
unsigned int j=0;

/*set interval timer to 5 ms*/
void set_Timer()
{
	memaddr *Timer;
	Timer = (memaddr *)INTERVALTMR; 
	*Timer = (unsigned int) 5000;
}

/*increment round variable*/
void add_round(unsigned int n)
{
	round+=n;
}

/*return a pointer to swap_queue*/
pcb_t* get_swap_queue()
{
	return swap_queue;
}

/*set the swap_queue pointer*/
void set_swap_queue(pcb_t *proc)
{
	swap_queue=proc;
}

/*Current_proc address*/
pcb_t* get_current()
{
	return Current_proc;
}  

/*increment round variable*/
void inc_round()
{
	round+=1;
}

/*set round variable to zero*/
void set_round_zero()
{
	round=0;
}

/*reset round command for scheduler activity*/
void reset_round()
{
	round=1;
}

/*number of process in the system*/
unsigned int get_proc_count()
{
	return procs_count;
}
 
/*number of blocked process*/
unsigned int get_sofblock_count()
{
	return softblock_count;
}

/*increment the process counter*/
void inc_proc_count()
{
	procs_count+=1;
}

/*increment blocked process counter*/
void inc_softblock_count()
{
	softblock_count+=1;
}

/*decrement process counter*/
void dec_proc_count()
{
	procs_count-=1;
}

/*decrement blocked process counter*/
void dec_softblock_count()
{
	softblock_count-=1;
}


/*true if proc ended its time slices*/
unsigned int equal_round_priority(pcb_t* proc)
{
	int tmp=11;
	
	if(proc->priority<0) tmp+=proc->priority*-1;
	else tmp-=proc->priority;
	
	if(tmp>round) return FALSE;
	else return TRUE;
}

/*ready queue address*/
pcb_t* get_ready_queue()
{
	return ready_queue;
}

/*set the ready queue pointer*/
void set_ready_queue(pcb_t *new_ptr)
{
	ready_queue=new_ptr;
}

/*set the current process to NULL*/
void set_current_NULL()
{
	Current_proc=NULL;
}

/*wait_state loop*/
void wait_state()
{
	while(1);;
}

/*get the ready queue head*/
pcb_t* head_ready_queue()
{
	return headThreadPQ(ready_queue);
}

/*add item to ready queue*/
void add_ready_queue(pcb_t *proc)
{
	insertPQ(&ready_queue,proc);
}

/*remove from ready queue*/
pcb_t* remove_ready_queue()
{	pcb_t* tmp=NULL; 
	
	
	tmp=removePQ(&ready_queue);
	tmp->t_next_l=NULL;
	tmp->t_next_r=NULL;
	return tmp;
}

/*out from ready queue*/
pcb_t* out_ready_queue(pcb_t *proc)
{
	pcb_t* tmp=NULL;
	
	tmp=outPQ(&ready_queue,proc);
	tmp->t_next_l=NULL;
	tmp->t_next_r=NULL;
	return tmp;
}


/*insert in the swap queue*/
void insert_swap_queue(pcb_t *proc)
{
	insertBackQ(&swap_queue,proc);

}

/*remove the first item in the swap queue*/
pcb_t* remove_swap_queue()
{
	pcb_t* tmp=NULL; 
	
	tmp=removeThreadQ(&swap_queue);
	tmp->t_next_l=NULL;
	tmp->t_next_r=NULL;
	return tmp;
}

/*remove proc from swap queue*/
pcb_t* out_swap_queue(pcb_t *proc)
{
	pcb_t * tmp=NULL;
	
	tmp=outThreadQ(&swap_queue,proc);
	tmp->t_next_l=NULL;
	tmp->t_next_r=NULL;
	return tmp;
}


/*experimental scheduler implementation*/
void scheduler()
{
	pcb_t *tmp=NULL;
	
	if(procs_count<=0) HALT(); /* "<=" paranoid */
	
	/* KAYA first invariant */
	if((ready_queue==NULL && swap_queue==NULL) && (procs_count != softblock_count))
		PANIC();
		
	if(ready_queue!=NULL)
	{
		/* current proc ended its timeslices */
		if(round==0)
		{
			if(Current_proc!=NULL)
			{
				tmp=remove_ready_queue();
				insert_swap_queue(tmp);
				tmp=NULL;
			}
				reset_round();
		}
		else   /*current proc have other timeslices*/
		{
			Current_proc=head_ready_queue();
			if(Current_proc == NULL) 
			{
				termprint("Current_proc NULL in scheduler, ma ready_queue NON vuota\n",0);
				PANIC();
			}
			set_all_int(&(Current_proc->proc_state),ON); 
			Current_proc->last_load=GET_TOD;
			LDST(&Current_proc->proc_state);
		}
	}
	else /*ready_queue is empty*/
	{
		ready_queue=NULL; /*paranoid */
		
		/* second deadlock  invariant for KAYA*/
		if( swap_queue==NULL && procs_count && softblock_count==0)
		{ 	
			termprint("deadlock\n",0);
			PANIC();
		}
		
		/*wait state! */
		if( swap_queue==NULL && procs_count && softblock_count && (procs_count == softblock_count))
		{
			wait.s_status=0;
			wait.s_cause=0;
			wait.s_pc = wait.t9 = (memaddr)wait_state;
			wait.s_status = wait.s_status| IEPBITON | CAUSEINTMASK;
			/* da wait_state() si esce solo con un interrupt...*/
			LDST(&wait);
		}
		/* ready_queue reloading*/
		while(swap_queue!=NULL)
		{
			tmp=remove_swap_queue();
			if(tmp==NULL) PANIC();
	
			add_ready_queue(tmp);
		}
	}
	scheduler(); /* recursive call to scheduler*/
}


/*scheduler initialization*/
void activate_scheduler(pcb_t * first_proc)
{
	if(activate_sched) PANIC();
	else
	{
		if(first_proc==NULL) PANIC();
		
		STST(&wait);
		wait.s_pc = wait.t9 = (memaddr)wait_state;
		wait.sp -=2048;
		activate_sched=1;
		ready_queue=NULL;
		insertThreadPQ(&ready_queue,first_proc);
		if(ready_queue==NULL) PANIC();
		inc_proc_count();
		set_Timer();
		reset_round();
		scheduler();
	}
}









