/**
 *  @file initial.c
 *  @author
 *  @brief System boot file
 *
 */

#include "../h/listx.h"
#include "../h/scheduler.h"
#include "../h/types10.h"
#include "../umps/uMPStypes.h"
#include "../umps/const.h"
#include "../umps/base.h"
#include "../e/pcb.e"
#include "../e/asl.e"
#include "../e/scheduler.e"
#include "../e/interrupts.e"
#include "../e/exceptions.e"
#include "/usr/include/uMPS/libumps.e"


pcb_t *running;
int dev_sem[4][8];
int dev_term[2][8];
int dev_status[4][8];
int dev_status_term[2][8];

/* initial function for test*/
extern void test();

int main()
{
	pcb_t *init=NULL;
	register int i;
	register int j;
	running = NULL;
	wait4pseudoclock = 0;


	for(i=0;i<4;i++)
	{
		for(j=0;j<8;j++)
		{
			dev_status[i][j]=0;
			dev_sem[i][j]=0;
		}
	}

	for(i=0;i<2;i++)
	{
		for(j=0;j<8;j++)
		{
			dev_status_term[i][j]=0;
			dev_term[i][j]=0;
		}
	}

	
	init_interrupt();
	init_exceptions();
	initPcbs();
	initSemd();
	init=allocPcb();
	if(init==NULL)	PANIC();

	STST(&init->p_state);
	init->p_state.status = 0x1000FF04;
	init->p_state.reg_t9 = init->p_state.pc_epc = (memaddr)test;
	init->p_state.reg_sp -=1024 ;
	pseudoclock= GET_TODLOW + SCHED_PSEUDO_CLOCK;
	init_Sched(init); /*start the scheduler and KAYA OS*/

	return 0;
}
