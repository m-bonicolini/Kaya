#include "h/const.h"
#include "h/types.h"
#include "e/libumps.e"
#include "h/kutil.h"
#include "e/threadq.e"
#include "e/asl.e"
#include "h/scheduler.h"
#include "h/system_call.h"

/*SYS1*/
void create_process()
{
	
	state_t *old_area=(state_t*) SYS_OLDAREA;
	pcb_t *current_proc=NULL;
	state_t * proc_state=NULL;
	pcb_t *son=NULL;
	state_t *son_state=NULL;

	son=allocTcb();
	if(son==NULL) 
	{
		/* return -1 in the v0 caller register */
		old_area->v0=-1;
		current_proc=get_current();
		proc_state=&current_proc->proc_state;
		mem_cpy((state_t*)old_area,proc_state,sizeof(state_t));
	}
    else
	{
		current_proc=get_current();
		proc_state=&current_proc->proc_state;
		inc_proc_count();
		if(((state_t*)old_area->a1)==NULL)PANIC();
		
		/* copy the old_area */
		son_state = &son->proc_state;
		mem_cpy((state_t*)old_area->a1,son_state,sizeof(state_t));
		son->priority=old_area->a2;
		/* inserisco the new process in the swap queue*/
		insert_swap_queue(son);
		insertChild(&(current_proc->child_queue),son);
		
		/* set v0 caller register to 0*/
		old_area->v0=0;
		mem_cpy((state_t*)old_area,proc_state,sizeof(state_t));
	}
}

/*recursive implemetation of SYS2*/
void terminate_process(pcb_t *proc)
{
	pcb_t *child_queue=NULL;
	pcb_t *child=NULL;
	pcb_t *tmp=NULL;

	
	child_queue=proc->child_queue;
	
	if(child_queue!=NULL)
	{
		child=removeQ(&child_queue);
		
		/* is blocked? */
		if(child->t_semAdd!=NULL)
		{
			child=outBlocked(child);
			if(child==NULL) PANIC();
			
			/*if is not device semaphore increment it*/
			if(!is_DevSem(child->t_semAdd)) 
				child->t_semAdd+=1;
			
			dec_softblock_count();
		}
		else
		{
			/* seek and remove from swap queue */
			tmp=out_swap_queue(child);
			
			/* the process is not in the swap queue */
			if(tmp==NULL)
			{
				child=out_ready_queue(child);
				
				if(child==NULL) PANIC(); /*the process is a ghost, panic()*/
			}
			tmp=NULL;
		}
		terminate_process(child); /* recursive call for children*/
		terminate_process(proc); /*recursive call for the caller process*/	
	}
	
	/* I'm the caller of a child? */
	if(proc==get_current())
	{
		proc=remove_ready_queue(); /*I'm the caller*/
	}
	else
	{
		if(child->t_semAdd!=NULL)
		{
			child=outBlocked(child);
			if(child==NULL) PANIC();
			
			if(!is_DevSem(child->t_semAdd))
				child->t_semAdd+=1;
				
			dec_softblock_count();
		}
		else
		{
			tmp=out_swap_queue(child);
			
			if(tmp==NULL)
			{
				child=out_ready_queue(child);
				
				if(child==NULL) PANIC();
			}
			tmp=NULL;
		}
	}
	freeTcb(proc);
	dec_proc_count();
	proc=NULL;
}


/*SYS3*/
pcb_t* verhogen(int *sem)
{
	pcb_t *blocked_proc=NULL;
		
	if(sem==NULL) PANIC();
	
	*sem += 1;
	
	if (*sem <= 0) 
	{
		blocked_proc =  removeBlocked(sem);
		if (blocked_proc == NULL) PANIC();
		dec_softblock_count();
		insert_swap_queue(blocked_proc); /* waked process go in the swap queue*/
		
		return blocked_proc;
	}
	return NULL;
}		

/*SYS4*/
void passern(int *sem)
{
	pcb_t *current_proc=NULL;

	
	if(sem==NULL) PANIC();
	
	*sem -= 1;

	if (*sem < 0) 
	{
		current_proc=remove_ready_queue();

		mem_cpy((state_t *)SYS_OLDAREA,&current_proc->proc_state, sizeof(state_t));
		
		next_istruction(&current_proc->proc_state);
		insertBlocked(sem, (pcb_t *) current_proc);
		inc_softblock_count();
		set_current_NULL();
	}
}

/*SYS5*/
void spec_exc_vector(unsigned int type, state_t* old_state, state_t* new_state) 
{
	pcb_t *current_process=get_current();
	
	if(current_process==NULL) PANIC();
	
	if(current_process->state_vector[type][OLDAREA] != NULL || 
	current_process->state_vector[type][NEWAREA] !=NULL)
	{
		terminate_process(current_process);
	}
	else
	{
		current_process->state_vector[type][OLDAREA]=old_state;
		current_process->state_vector[type][NEWAREA]=new_state;
		mem_cpy((state_t*)SYS_OLDAREA,&current_process->proc_state,sizeof(state_t));
		next_istruction(&current_process->proc_state);
	}		
}

/*SYS6*/
void get_cputime(cpu_t now)
{
	pcb_t *current_process=get_current();
	
	current_process->cpu_time+=(now - current_process->last_load);
	current_process=NULL;
	
}

/*SYS7*/
void waitclock()
{	
	passern((int*)pseudo_clock_addr());
}

/*SYS8*/
/*int*/ void wait_io(unsigned int intline, unsigned int devnum, unsigned int flag)
{
	
	pcb_t * current_proc=NULL;
	
	current_proc=get_current();
	
	if(intline >7) PANIC();
	
	
	if(current_proc==NULL)
	{
		PANIC();
	}
	
	if (intline < 7)	/*device non terminale*/ 
	{
		passern((int*)&dev_sem[intline-3][devnum]);
	}
	else /*device terminale*/
	{
		
		passern((int *)&dev_term[flag][devnum]);
		
	}
	
}








