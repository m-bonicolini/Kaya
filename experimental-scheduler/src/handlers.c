#include "h/const.h"
#include "h/types.h"
#include "e/libumps.e"
#include "h/kutil.h"
#include "e/threadq.e"
#include "e/asl.e"
#include "h/scheduler.h"
#include "h/system_call.h"
#include "h/handlers.h"


/*old area handlers pointer*/
HIDDEN state_t *new_pgmTrap=(state_t *)PTRAP_NEWAREA;
HIDDEN state_t *new_Bk=(state_t *)SYS_NEWAREA;
HIDDEN state_t *new_tlb=(state_t *)TLB_NEWAREA;
HIDDEN state_t *new_ints=(state_t *)INT_NEWAREA;
HIDDEN unsigned int init_h=FALSE;
HIDDEN unsigned int time_slices=0;
HIDDEN unsigned int timer=0;


HIDDEN unsigned int get_int_devices(unsigned int *int_line_devmap)
{
	unsigned int devnum = 0;
	unsigned int tmp=0;
	
	if(int_line_devmap==NULL) PANIC();

	tmp = *int_line_devmap;
	
	devnum = 0;
		
	
	while (!(tmp & 0x1)) 
	{
		devnum++;
		tmp = tmp >> 1;	
	}
	
	return devnum;
}


HIDDEN int int_ack(device_t *device)
{
	if ((device->d_status & 0xFF) == 1) 
	{
		device->d_command = (device->d_command & 0xFFFFFF00) | 0x1; /*ack*/
		return 1; 
	}
	else	
	{
		device->d_command = (device->d_command & 0xFFFFFF00) | 0x0;/*reset*/
		return 0;
	}
}


HIDDEN void switch_ints_cause(state_t *proc)
{
	unsigned int i=2;

	pcb_t* tmp=NULL;
	
	if(is_int_line(0,proc) || is_int_line(1,proc)) PANIC();
	
	/*solve all interrupts*/
	for(i=2;i<8;i++)
	{
		if(is_int_line(i,proc))
		{
			switch(i)
			{
				case 2:
				{	
					time_slices += 1;
					if (time_slices == 20) 
					{
						while (get_pseudo_clock()<0) 
						{
							tmp=verhogen((int*)pseudo_clock_addr());
						}
						time_slices = 0;
					}
					timer=1;
					set_Timer();
				    inc_round();
				}break;
				case 3:
				{
					device_t *disk=NULL;
					unsigned int devnum = 0;
			
				#ifdef BOHOS_DEBUG
					termprint("Interrupt disco\n",0);		
				#endif
				
					disk = (device_t *) INTLINE_ADDR(3,0);
					devnum = get_int_devices((unsigned int *)DISK_ADDR);/* bitmap offset*/		
					
					disk += devnum;
					
					tmp=verhogen((int*)&dev_sem[0][devnum]);
				
					if(tmp==NULL) dev_status[0][devnum]=disk->d_status;
					else tmp->proc_state.v0=disk->d_status;
					int_ack(disk);
					
				}break;
				case 4:
				{
					device_t *tapes=NULL;
					unsigned int devnum = 0;
					
					
					tapes = (device_t *) INTLINE_ADDR(4,0);
					devnum = get_int_devices((unsigned int*)TAPES_ADDR);
					
					tapes += devnum; 
					
					tmp=verhogen((int*)&dev_sem[0][devnum]);
				
					if(tmp==NULL) dev_status[0][devnum]=tapes->d_status;
					else tmp->proc_state.v0=tapes->d_status;
					
					int_ack(tapes);
				}break;
				case 5:
				{
					device_t *net=NULL;
					unsigned int devnum = 0;
					
					
					net = (device_t *) INTLINE_ADDR(5,0);
					devnum = get_int_devices((unsigned int*)NET_ADDR);
					
					net += devnum; 
					
					tmp=verhogen((int*)&dev_sem[0][devnum]);
				
					if(tmp==NULL) dev_status[0][devnum]=net->d_status;
					else tmp->proc_state.v0=net->d_status;
					
					int_ack(net);
				}break;
				case 6:
				{
					device_t *print=NULL;
					unsigned int devnum = 0;
					
					termprint("printer \n",0);
					
					print = (device_t *) INTLINE_ADDR(6,0);
					devnum = get_int_devices((unsigned int*)NET_ADDR);	
					
					print += devnum; 
					
					tmp=verhogen((int*)&dev_sem[0][devnum]);
				
					if(tmp==NULL) dev_status[0][devnum]=print->d_status;
					else tmp->proc_state.v0=print->d_status;
					
					int_ack(print);
				}break;
				case 7:
				{
					device_t *term=NULL;
					unsigned int devnum = 0;
					
					
					term = (device_t *) INTLINE_ADDR(7,0);
					devnum = get_int_devices((unsigned int*)TERMINAL_ADDR);		
					
					term += devnum; 
					
					if ((term->d_status & 0xFF) ==  5) 
					{
						tmp=verhogen((int*)&dev_term[1][devnum]);
						if(tmp==NULL)
							dev_status_term[1][devnum]=TERM_RX_STATUS(devnum);
						else tmp->proc_state.v0=TERM_RX_STATUS(devnum);
						
						term->d_command = (term->d_command & 0xFFFFFF00) |0x00000001; /*ack*/	

					}
					else
					{
						if((term->d_status & 0xFF)!=1)
						{
							term->d_command = (term->d_data1 & 0xFFFFFF00)|0x1;/*reset*/
						}
					}

					if ((term->d_data0 & 0xFF) == 5 ) 
					{	
						tmp=verhogen((int*)&dev_term[0][devnum]);
						if(tmp==NULL)
							dev_status_term[0][devnum]=TERM_TX_STATUS(devnum); 
						else tmp->proc_state.v0=TERM_TX_STATUS(devnum); 
						
						term->d_command = (term->d_command & 0xFFFFFF00) |1;
												
						term->d_data1 = (term->d_data1 & 0xFFFFFF00) | 1; /*ack*/
						

					}
					else
					{			
						if((term->d_status & 0xFF)!=1)
							term->d_data1 = (term->d_data1 & 0xFFFFFF00)| 1;/*reset*/
						
					}
				}break;
				default:
				{
					PANIC();
				}break;
			}
		}	
	}
}

/*rom interrupt handler*/
HIDDEN void ints_handler()
{
	state_t *old_area=NULL;
	pcb_t *current_proc=get_current();

	if(current_proc!=NULL) get_cputime(GET_TOD);
	
	old_area=(state_t*) INT_OLDAREA;
	if(old_area==NULL) PANIC(); 
	
	switch_ints_cause(old_area); 
	
	if(current_proc==NULL) 
	{
		scheduler();
	}
	
	if(equal_round_priority(current_proc) && timer) 
	{
		timer=0;

		mem_cpy(old_area,&current_proc->proc_state,sizeof(state_t));
		set_round_zero();
		scheduler();
	}
	else
	{
		if(timer) timer=0;
		
		mem_cpy(old_area,&current_proc->proc_state,sizeof(state_t));
		scheduler();	
	}
}

 
HIDDEN void pass_up(unsigned int type, state_t *old_area, pcb_t *process)
{
	state_t *old_state = process->state_vector[type][OLDAREA];
	state_t *new_state = process->state_vector[type][NEWAREA];
	
	if(process==NULL || old_area==NULL) PANIC();
	
	if(type>2) PANIC();
	
	if (old_state==NULL || new_state == NULL)
	{
		terminate_process(process);
		set_round_zero();
		scheduler();
	} 
	else 
	{	
		mem_cpy((state_t *) old_area, old_state, 20);
		LDST(new_state);
	}
}

HIDDEN void sys_bp_handler()
{
	register state_t *old_area=NULL;
	state_t *old_pgm_area=NULL;
	state_t *new_area=NULL;
	unsigned int cause=0;
	unsigned char* pointer=NULL;
	pcb_t *current_proc=NULL;
	unsigned int type=0;
	state_t * tmp_state=NULL;
	
	
	current_proc=get_current();
	
	if(current_proc!=NULL) get_cputime(GET_TOD);
	
	
	
	old_area=(state_t *)SYS_OLDAREA;
	if(old_area==NULL) PANIC();
	
	cause=get_cause(old_area);
	
	if((cause & 0xFC)==0x00000020)
	{
		
		if(!is_kernel_mode(old_area))
		{
			if((old_area->a0>=1) || (old_area->a0<=8))
			{
				old_pgm_area=(state_t*) PTRAP_OLDAREA;
				mem_cpy(old_area,old_pgm_area,sizeof(state_t));
				
				pointer= (unsigned char*)&(old_pgm_area->s_cause);
				pointer[3]|=8;
				pointer[3]|=32;
				pointer=NULL;
				
				LDST((state_t*)PTRAP_NEWAREA);
			}
			else
				pass_up(SYSTRAP,old_area,current_proc);
		}
		/*kernel mode*/
		else  
		{
			switch(old_area->a0)
			{
				case CREATETHREAD:
				{
					create_process();
					next_istruction(&current_proc->proc_state);
					
					scheduler();
				}break;
				case TERMINATETHREAD:
				{
					terminate_process(current_proc);
					current_proc=NULL;
					set_current_NULL();
					set_round_zero();
					scheduler();
				}break;
				case VERHOGEN: 
				{
					verhogen((int*)old_area->a1);
					next_istruction(old_area);
					mem_cpy(old_area,&current_proc->proc_state,sizeof(state_t));
					scheduler();
				}break;
				case PASSERN:
				{
					passern((int*)old_area->a1);
					if(get_current()!=NULL)
					{
						next_istruction(old_area);
						
						mem_cpy(old_area,&current_proc->proc_state,sizeof(state_t));
					}
					else  set_round_zero();
					scheduler();
					
				}break;
				case SPECTRAPVEC:
				{
					type=(unsigned int)(((state_t*)SYS_OLDAREA)->a1);
					old_area=(state_t*)(((state_t*)SYS_OLDAREA)->a2);
					new_area=(state_t*)(((state_t*)SYS_OLDAREA)->a3);
					spec_exc_vector(type,old_area,new_area);

					scheduler();
				}break;
				case GETCPUTIME:  
				{   
					tmp_state=&current_proc->proc_state;
					mem_cpy((state_t*)SYS_OLDAREA,tmp_state,sizeof(state_t));
					next_istruction(&current_proc->proc_state);

					current_proc->proc_state.v0=current_proc->cpu_time;
					scheduler();
				}break;
				case WAITCLOCK: 
				{
					waitclock();
					if(get_current()!=NULL)
					{
						next_istruction(old_area);
						mem_cpy(old_area,&current_proc->proc_state,sizeof(state_t));
					}
					else set_round_zero();

					scheduler();
				}break;
				case WAITIO:
				{
					unsigned int int_line=0;
					unsigned int dev_num=0;
					unsigned int flag=0;
					state_t *wait_old_area=(state_t*)SYS_OLDAREA;

					int_line=(unsigned int)wait_old_area->a1;
					dev_num=(unsigned int)wait_old_area->a2;
					flag=(unsigned int)wait_old_area->a3;
					wait_io(int_line,dev_num,flag);
					
					if(get_current()!=NULL)
					{
						old_area->v0=dev_status_term[flag][dev_num];
						next_istruction(old_area);
						mem_cpy(old_area,&current_proc->proc_state,sizeof(state_t));
					}
					else set_round_zero();
					
					scheduler();
				}break;
				default:
				{
					pass_up(SYSTRAP,old_area,current_proc);
				}break;
			}
			
		}	
	}
	
	if((cause & 0xFC)==0x00000024)/*caso break point*/
	{
		if(!is_kernel_mode(old_area) && ((old_area->a0>=1) || (old_area->a0<=4)))
		{
				old_pgm_area=(state_t*) PTRAP_OLDAREA;
				mem_cpy(old_area,old_pgm_area,sizeof(state_t));
				
				pointer=(unsigned char*)&(old_pgm_area->s_cause);
				pointer[3]|=8;
				pointer[3]|=32;
				pointer=NULL;
				
				LDST((state_t*)PTRAP_NEWAREA);
		}
		else pass_up(SYSTRAP,old_area,current_proc);
	}
}

HIDDEN void pgm_handler()
{
	/*LOL :D*/
	get_cputime(GET_TOD);
	pass_up(PROGTRAP,(state_t*)PTRAP_OLDAREA,get_current());
}

HIDDEN void tlb_handler()
{
	/*ROTFLOL :D*/
	get_cputime(GET_TOD);
	pass_up(TLBTRAP,(state_t*)TLB_OLDAREA,get_current());
}


HIDDEN void init_Handlers()
{	
	new_ints-> s_pc = new_ints->t9 = (memaddr)ints_handler;
	new_ints-> sp = RAMTOP-(4*1024);
   	new_ints-> s_status = (unsigned int)0x10000000; /*set up CP0*/
	
	new_Bk-> s_pc = new_Bk->t9 = (memaddr)sys_bp_handler;
	new_Bk-> sp = RAMTOP-(5*1024);
   	new_Bk-> s_status = (unsigned int)0x10000000;
	
	new_tlb-> s_pc = new_tlb->t9 = (memaddr)tlb_handler;
	new_tlb-> sp = RAMTOP-(6*1024);
   	new_tlb-> s_status = (unsigned int)0x10000000;
	
	new_pgmTrap-> s_pc = new_pgmTrap->t9 = (memaddr)pgm_handler;
	new_pgmTrap-> sp = RAMTOP-(7*1024);
   	new_pgmTrap->s_status = (unsigned int)0x10000000;
}

int activate_handlers()
{
	int i=0;
	int j=0;
	if(init_h) return FALSE;
	else
	{
		init_h=TRUE;
		init_Handlers();
	}
	
	for(i=0;i<4;i++)
	{
		for(j=0;j<8;j++)
		{
			dev_status[i][j]=0;
		}
	}
	
	for(i=0;i<2;i++)
	{
		for(j=0;j<8;j++)
		{
			dev_status_term[i][j]=0;
		}
	}
	return TRUE;
}
