#include "const.h"
#include "types.h"

#define TRANSMITTED			5
#define ACK				1
#define PRINTCHR			2
#define CHAROFFSET			8
#define STATUSMASK			0xFF
#define TERM0ADDR   			0x10000250

#define	INTLINE0			0
#define	INTLINE1			1
#define	TIMER				2
#define DISKS				3
#define	TAPES				4
#define NET				5
#define PRINT				6
#define TERM				7


#define TERM_DEVREG_BASE		0x10000250
#define DEVREG_BASE 			0x10000050

#define INTLINE_ADDR(intln,devnum)	   (DEVREG_BASE+((intln-3)*0x80)+(devnum*0x10))
#define DEVREG_STATUS(intln,devnum) 	   ((device_t *)INTLINE_ADDR(intln,devnum))->d_status 
#define DEVREG_COMMAND(intln,devnum)	   ((device_t *)INTLINE_ADDR(intln,devnum))->d_command 
#define DEVREG_DATA0(intln,devnum) 	   ((device_t *)INTLINE_ADDR(intln,devnum))->d_data0 
#define DEVREG_DATA1(intln,devnum)         ((device_t *)INTLINE_ADDR(intln,devnum))->d_data1 

#define TERM_RX_STATUS(devnum) 			((device_t *) (TERM_DEVREG_BASE+devnum*sizeof(device_t)))->t_recv_status 
#define TERM_RX_CMD(devnum) 			((device_t *) (TERM_DEVREG_BASE+devnum*sizeof(device_t)))->t_recv_command 
#define TERM_TX_STATUS(devnum) 			((device_t *) (TERM_DEVREG_BASE+devnum*sizeof(device_t)))->t_transm_status 
#define TERM_TX_CMD(devnum) 			((device_t *) (TERM_DEVREG_BASE+devnum*sizeof(device_t)))->t_transm_command 

#define INTERRUPT_DEVMAP_BASE		0x1000003c
#define DISK_ADDR			0x1000003c
#define TAPES_ADDR			0x10000040
#define NET_ADDR			0x10000044
#define PRINT_ADDR			0x10000048
#define TERMINAL_ADDR			0x1000004c

#define SYS_OLDAREA             	0x20000348
#define SYS_NEWAREA             	0x200003D4
#define INT_OLDAREA             	0x20000000
#define INT_NEWAREA             	0x2000008C
#define PTRAP_OLDAREA           	0x20000230
#define PTRAP_NEWAREA           	0x200002BC
#define TLB_OLDAREA             	0x20000118
#define TLB_NEWAREA             	0x200001A4


/*tabella  di CAUSE*/

#define INT_CAUSE 	 0
#define MOD_CAUSE 	 1
#define TLBL_CAUSE	 2
#define TLBS_CAUSE	 3
#define ADEL_CAUSE	 4
#define ADES_CAUSE	 5
#define IBE_CAUSE 	 6
#define DBE_CAUSE 	 7
#define SYS_CAUSE 	 8
#define BP_CAUSE  	 9
#define RI_CAUSE  	10
#define CPU_CAUSbE 	11
#define OV_CAUSE  	12
#define BDPT_CAUSE 	13
#define PTMS_CAUSE 	14



#define RAMTOP		0x20040000

int dev_sem[4][8];
int dev_term[2][8];
unsigned int dev_status[4][8];
unsigned int dev_status_term[2][8];

extern devreg termstat(memaddr * stataddr);

extern unsigned int termprint(char * str, unsigned int term);

extern int sumbitmap(unsigned int semAdd_to_int);

extern int mem_set(void* p,unsigned char value, unsigned int s_size);

extern void mem_cpy(void *source,void* dest,unsigned int size);

extern void swap_pointer(void **p,void **q);

extern void set_interrupt_OFF(state_t *proc);

extern int is_kernel_mode(state_t *proc);

extern unsigned int is_int_line(unsigned int index,state_t *proc);

extern void set_int_lineON(unsigned int index,state_t *proc);

extern void set_int_lineOFF(unsigned int index,state_t *proc);

extern void set_all_int(state_t *proc,unsigned int flag);

extern void next_istruction(state_t *proc);

extern unsigned int get_cause(state_t *proc);

extern void inc_pseudo_clock();

extern void dec_pseudo_clock();

extern int get_pseudo_clock();

extern int* pseudo_clock_addr();

extern unsigned int is_DevSem(int *sem); 
