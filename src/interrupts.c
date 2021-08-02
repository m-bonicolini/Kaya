/* SPDX-License-Identifier: GPL-2.0 */
/**
 *  @brief Implementation of the KAYA handlers
 *  
 *  This file is the source code of the Interrupt Handler, 
 * 	The only public function is defined in the interrupts.e interface 
 *  and it is the init_exceptions().
 *
 * 
 *  @see interrupts.e for info about the implementation 
 *  @copyright  GNU General Public License, version 2 
 *  @file
 *
 **/
#include "../umps/const.h"
#include "../h/types10.h"
#include "../umps/uMPStypes.h"
#include "../umps/base.h"
#include "../e/scheduler.e"
#include "../e/interrupts.e"
#include "../e/pcb.e"
#include "../e/asl.e"

#include "/usr/include/uMPS/libumps.e"


/**
  * @brief interrupt timer line interrupt
*/
#define TIMER 2

/**
  * @brief terminal line interrupt
*/
#define TERMINAL 7


#define IS_INIT_HANDLER  (i_hand == TRUE);


/**
  * @brief timer quantum
*/
#define TIMERLIMIT 500


/**
  * @brief devices semaphore
*/
extern int dev_sem[4][8];

/**
  * @brief terminal semaphore
*/
extern int dev_term[2][8];

/**
  * @brief devices status
*/
extern int dev_status[4][8];

/**
  * @brief terminal status
*/
extern int dev_status_term[2][8];

extern struct list_head readyQ;
extern pcb_t* running;
extern  pcb_t* verhogen(int* sem);
extern pcb_t waiting;


HIDDEN state_t *Ints=(state_t *)INT_NEWAREA;

/**
  * @brief Get the raised interrupt with high priority 
  * @param void
  * @return int the raised interrupt line
 */
HIDDEN int get_pending_line();

/**
  * @brief The Interrupt Handler
  * @param void
  * @return void.
 */
HIDDEN void ints_handler();


/**
  * @brief The function take a raised line and return the first ready 
  * device for this line in the bitmap
  * @param uint*  a raised line checked with get_pending_line()
  * @return uint the device number for the interrupt
 */
HIDDEN unsigned int get_int_devices(unsigned int *int_line_devmap);


void init_interrupt(void);

HIDDEN void ints_handler()
{
	time(GET_TODLOW);
	int lineNum = get_pending_line();


	switch(lineNum)
	{
	    case TIMER:
		{
			if(pseudoclock <= GET_TODLOW)/
			{
				pseudoclock += SCHED_PSEUDO_CLOCK; 
				while(wait4pseudoclock<0) 
					verhogen(&wait4pseudoclock); 
			}

			if(running != &waiting) 
			{
				if(running->remaining > TIMERLIMIT) 
					extern_scheduler(INTSCHED,TRUE); 

				state_t_copy((state_t*)INT_OLDAREA, (state_t*)&(running->p_state)); 
				insertProcQ(&readyQ,running);
			}

			extern_scheduler(CONTEXTSWITCH,TRUE);
		}
		 break;
	    case TERMINAL:
		{
			termreg_t *term=NULL;
			unsigned int devnum = 0;
			pcb_t *tmp;
			term = (termreg_t *) INTLINE_ADDR(7,0);
			devnum = get_int_devices((unsigned int*)TERMINAL_ADDR);
			term += devnum; 

			if ((term->recv_status & 0xFF) ==  5 || (term->transm_status & 0xFF) == 5 )
			{	
				if ((term->transm_status & 0xF) == 5 )
				{
					if ((term->transm_status & 0xFF) == 5 )
					{

						tmp=verhogen((int*)&dev_term[0][0]);
						if(tmp!=NULL)
							tmp->p_state.reg_v0=TERM_TX_STATUS(devnum);
						else
							dev_status_term[0][0]=TERM_TX_STATUS(devnum);

						term->transm_status = (term->transm_status & 0xFFFFFF00) |1;

						term->transm_command = 1 ; /*ack*/

					}
					else
						term->transm_command=0;


					extern_scheduler(INTSCHED,TRUE);

				}
				else
				{	
					tmp=verhogen(&dev_term[1][devnum]);
					dev_status_term[1][devnum]=TERM_RX_STATUS(devnum);
					tmp->p_state.reg_v0=TERM_RX_STATUS(devnum);
					term->recv_command = (term->recv_command & 0xFFFFFF00) |0x00000001; /*ack*/
				}

			}
			else
				term->transm_command=0;

		}
	    default: /* other devices*/
	    		{ 
	    			dtpreg_t *devices=NULL;
	    			unsigned int devnum = 0;
	    			pcb_t *tmp=NULL;
	    			devices = (dtpreg_t *) INTLINE_ADDR(lineNum,0);
	    			devnum = get_int_devices((unsigned int*)devices);/*offset della bitmap*/

	    			devices += devnum; /*seleziono il devices opportuno*/
	    			if ((devices->status & 0xFF) == 5 )
	    			{
	    				tmp=verhogen((int*)&dev_sem[lineNum][devnum]);
	    				if(tmp!=NULL)
	    					tmp->p_state.reg_v0=devices->status & 0xFF;
	    				else
	    					dev_status[lineNum][devnum]=devices->status & 0xFF;

	    				devices->status = (devices->status & 0xFFFFFF00) |1;
	    				devices->command = 1 ; /*ack*/

	    			}
	    		}break;


	}
}

HIDDEN int get_pending_line()
{
    register int i;
    i=INT_TIMER;
	while ((i<=INT_TERMINAL) && (!CAUSE_IP_GET( ((state_t*)INT_OLDAREA)->cause, i)))
			i++;
	
    return i;
}


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


 void init_interrupt()
 {
	Ints-> pc_epc = Ints->reg_t9 = (memaddr)ints_handler;
	Ints-> reg_sp = RAMTOP-(4*1024);
	Ints-> status = (unsigned int)0x10000000; 
 }


