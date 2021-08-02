#include "h/types.h"
#include "h/const.h"
#include "h/kutil.h"
#include "e/libumps.e"

HIDDEN int pseudo_clock=0;

/*terminal status mask*/
devreg termstat(memaddr * stataddr)
{
	return((*stataddr) & STATUSMASK);
}

/*print on terminal*/
unsigned int termprint(char * str, unsigned int term)
{
	register memaddr * statusp;
	register memaddr * commandp;
	register devreg stat;
	register devreg cmd;
	register unsigned int error = FALSE;
	
	if (term < DEVPERINT)
	{
		/* terminal is correct */
		/* compute device register field addresses */
		statusp = (devreg *) (TERM0ADDR + (term * DEVREGSIZE) + (TRANSTATUS * DEVREGLEN));
		commandp =(devreg *) (TERM0ADDR + (term * DEVREGSIZE) + (TRANCOMMAND * DEVREGLEN));
		
		/* test device status */
		stat = termstat(statusp);
		if (stat == READY || stat == TRANSMITTED)
		{
			/* device is available */
			
			/* print cycle */
			while (*str != EOS && !error)		
			{
				cmd = (*str << CHAROFFSET) | PRINTCHR;
				*commandp = cmd;
				

				/* busy waiting */
				stat = termstat(statusp);
				while (stat == BUSY)
					 stat = termstat(statusp);
				
				/* end of wait */
				if (stat != TRANSMITTED)
					error = TRUE;
				else
					/* move to next char */
					str++;
			} 
		}
		else
			/* device is not available */
			error = TRUE;
	}
	else
		/* wrong terminal device number */
		error = TRUE;

	return (!error);		
}

/*count the number of bit setted to 1 in a word*/
int sumbitmap(unsigned int semAdd_to_int)
{
	int res=0;
	int i=31;
	unsigned int value=semAdd_to_int;	
	
	for(i=31;i>-1;i--)
	{
		if(value & 1<<i) res+=1;
	}
	return res;
}


/*memset implementation for kaya*/
int mem_set(void* p,unsigned char value, unsigned int s_size)
{
	unsigned char *bt=NULL;
	register unsigned int i=0;
	
	if(p==NULL) return 1;
	
	bt=(unsigned char*) p;
	
	for(i=0;i<s_size;i++)	
		bt[i]=value;
	
	bt=NULL;
	return 0;
}

/*memcpy implementation for kaya*/
void mem_cpy(void *source,void* dest,unsigned int size)
{
	unsigned char *bt0=NULL;
	unsigned char *bt1=NULL;
	unsigned int i=0;
	
	if(dest==NULL) PANIC();
	
	bt0=(unsigned char*)dest;
	bt1=(unsigned char*)source;
	
	for(i=0;i<size;i++)
		bt0[i]=bt1[i];
	
	bt0=NULL;
	bt1=NULL;
}

void swap_pointer(void **p,void **q)
{
	register void* tmp=NULL;
	
	if(p==NULL || q==NULL || *q==NULL || *p==NULL) PANIC();
	
	tmp=*p;
	*p=*q;
	*q=tmp;
	
	tmp=NULL;
	
}


/*set off interrupt for a processor state*/
void set_interrupt_OFF(state_t *proc)
{
	register unsigned char mask=251;
	register unsigned char *p=NULL;

	if(proc==NULL) PANIC();
	
	p=(unsigned char*)&(proc->s_status);
	
	p[3]&=mask;

	p=NULL;
	
}

/*check the kernel mode*/
int is_kernel_mode(state_t *proc)
{
	if(proc==NULL) PANIC();
	
	if(proc->s_status & KUPBITON) return FALSE;
	
	return TRUE;
}


/*Check one interrupt line*/
unsigned int is_int_line(unsigned int index,state_t *proc)
{
	unsigned int cause=0;
	
	cause=proc->s_cause;
	switch(index)
	{
		case 0:
		{
		    	cause&=0x100;
		}break;
		case 1:
		{
				cause&=0x200;
		}
		case 2:
		{
				cause&=0x400;
		}break;
		case 3:
		{
				cause&=0x800;
		}break;
		case 4:
		{
				cause&=0x1000;
		}break;
		case 5:
		{
				cause&=0x2000;
		}break;
		case 6:
		{
				cause&=0x4000;
		}break;
		case 7:
		{
				cause&=0x8000;
		}break;
		default:
		{
			PANIC();
		}break;
	}
	return cause;
}

/*set one inline for a processor state*/
void set_int_lineON(unsigned int index,state_t *proc)
{	
	if(proc==NULL) PANIC();
	
	switch(index)
	{
		case 0:	/*interrupt line0*/
		{
			proc->s_status|=0x100;
			
		}break;
		case 1:	/*interrupt line1*/
		{
			proc->s_status|=0x200;
		}break;
		case 2:	/*timer*/
		{
			proc->s_status|=0x400;
		}break;
		case 3:/*disk*/
		{
			proc->s_status|=0x800;
		}break;
		case 4:/*tapes*/
		{
			proc->s_status|=0x1000;

		}break;
		case 5:/*net*/
		{
			proc->s_status|=0x2000;
		}break;
		case 6:/*print*/
		{
			proc->s_status|=0x4000;
		}break;
		case 7:/*term*/
		{
			proc->s_status|=0x8000;
		}break;
		default:
		{
			PANIC();
		}break;
	}
}

/*select one interrupt line from offset*/
void set_int_lineOFF(unsigned int index,state_t *proc)
{	
	unsigned char mask=255-(1<<index);
	unsigned char* p=NULL;
	
	if(proc==NULL) PANIC();
	if(index>7) PANIC();
	
	p=(unsigned char*)&(proc->s_status);
	p[3]&=mask;
	
	p=NULL;

}

/*sett al interrupt line with on/off*/
void set_all_int(state_t *proc,unsigned int flag)
{
	register int i=0;
	
	if(flag)
	{
		proc->s_status|= IEPBITON | CAUSEINTMASK;
	}
	else
	{
		set_interrupt_OFF(proc);
	
		for(i=0;i<8;i++)
			set_int_lineOFF(i,proc);
	}
}


/*progra counter point to next instruction*/
void next_istruction(state_t *proc)
{
	if(proc==NULL) PANIC();
	
	proc->t9=proc->s_pc+=4;
}

/*return the interrupt cause of a processor state*/
unsigned int get_cause(state_t *proc)
{	
	if(proc==NULL) PANIC();
	
	return proc->s_cause;
	
}

/* check if sem is a device semaphore */
unsigned int is_DevSem(int *sem) 
{
	int i,j = 0;
	
	/* device semaphore */
	for(i=0;i<4;i++) 
	{
		for(j=0;j<8;j++) 
		{
			if(&dev_sem[i][j] == sem)
				return 1;
		}
	}
	
	/* terminal semaphore */
	for(i=0;i<2;i++) 
	{
		for(j=0;j<8;j++) 
		{
			if(&dev_term[i][j] == sem)
				return 1;
		}
	}
	return 0;
}

/*increment pseudo clock*/
void inc_pseudo_clock()
{
	pseudo_clock+=1;
}

/*decrement pseudo clock*/
void dec_pseudo_clock()
{
	pseudo_clock-=1;
}

/*get the pseudo clock value*/
int get_pseudo_clock()
{
	return pseudo_clock;
}

/*get the pseudo clock address*/
int* pseudo_clock_addr()
{
	return &pseudo_clock;
}
