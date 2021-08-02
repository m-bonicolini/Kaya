#ifndef _TYPES10_H
#define _TYPES10_H
#include "../umps/base.h"
#include "../umps/const.h"
#include "../umps/uMPStypes.h"
#include "listx.h"

typedef struct pcb_t
{
	/*process queue fields */
	struct list_head	p_next;

	/*process tree fields */
	struct list_head	p_child,p_sib;
	struct pcb_t  *p_prnt;

	/* low level info */
	state_t p_state;
	S32 *p_semAdd;
	unsigned int cpu_time; 
	unsigned int start_time; 
	unsigned int pid; 
	U32 remaining; 
	U8 flag[3]; 
	state_t* SPCVEC[6];
} pcb_t;

typedef struct semd_t
{
	struct list_head	s_next;
	S32			*s_semAdd;
	struct list_head	s_procQ;
	U8 indice;
} semd_t;

#endif
