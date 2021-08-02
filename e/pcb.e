/* SPDX-License-Identifier: GPL-2.0 */
/**
 *  @brief Interface header for the process queue handling. 
 *  
 *  The file define the process queue handling interface. 
 *  As constraint it must use the linux kernel linked list because some function
 *  signature require it in the arguments definition. 
 *
 *	@note It should be better to hide the linux linked list usage 
 *  with information hiding
 * 
 *  @see pcb.c for info about the implementation and suggestion to 
 *  further development
 *  @copyright  GNU General Public License, version 2 
 *  @file
 *
 **/

#ifndef PCB_E
#define PCB_E

#include "../h/listx.h"
#include "../h/types10.h"

/**
*  @brief Free an allocate process and insert it in the free list
*  @param pcb_t* the process to free
*  @return void
*  @warning Call this functiion before initPcbs() or with a NULL arg raise a PANIC()
*/
void freePcb(pcb_t *p);

/**
*  @brief Alloc a process in the system
*  @param pcb_t* the allocated process
*  @return void
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
pcb_t *allocPcb(void);

/**
*  @brief This function is to run only once before the use of any function in this module.
*  It init the data struct of the module
*  @param void
*  @return void
*  @warning Call this function more than once raise a PANIC()
*/
void initPcbs(void);

/**
*  @brief Make an empty list from list_head pointer
*  @param struct list_head* a pointer to the list to make empty
*  @return void
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
void mkEmptyProcQ(struct list_head *emptylist);

/**
*  @brief Check if a list head is empty
*  @param struct list_head* a pointer to the list to check
*  @return 1 if the list is empty, 0 otherwise
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
int emptyProcQ(struct list_head *head);

/**
*  @brief Insert a process in a list_head. The policy is FIFO
*  @param struct list_head* a pointer to the list where insert the process
*  @param pcb_t* a pointer to the process to insert
*  @return void
*  @warning Call this functiion before initPcbs() or with NULL args raise a PANIC()
*/
void insertProcQ(struct list_head *head, pcb_t *p);

/**
*  @brief Remove the first list_head process. The order is FIFO 
*  @param struct list_head* pointer to the list where remove the process
*  @return pcb_t* the removed process, NULL if the list is empty
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
pcb_t *removeProcQ(struct list_head *head);

/**
*  @brief Remove a process from a list_head 
*  @param struct list_head* pointer to the list where remove the process
*  @param pcb_t* the process to remove
*  @return pcb_t* the removed process, NULL if the list is empty
*  @warning Call this functiion before initPcbs() or with NULL args raise a PANIC()
*/
pcb_t *outProcQ(struct list_head *head, pcb_t *p);

/**
*  @brief Get the first list_head process. The order is FIFO 
*  @param struct list_head* the target list where get the process
*  @return pcb_t* the first process, NULL if the list is empty
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
pcb_t *headProcQ(struct list_head *head);

/**
*  @brief Get the process associated to a pid in the system 
*  @param int the pid value
*  @return pcb_t* the seeked process
*  @note the pid value must be >=0
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
pcb_t *resolvePid(int pid);


/**
*  @brief Count the total allocated process in the system by allocPcb()
*  @param void
*  @return int the number of allocated process
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
int count_actives(void);

/**
*  @brief Check if a process has some child
*  @param pcb_t* the process to check
*  @return 1 if the process have some child, 0 otherwise
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
int emptyChild(pcb_t *p);

/**
*  @brief Insert a process in the parent child list.
*  @param pcb_t* the parent process
*  @param pcb_t* the child process
*  @return void
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
void insertChild(pcb_t *parent, pcb_t *child);

/**
*  @brief Remove the first child a process. The order is FIFO
*  @param pcb_t* the parent process
*  @return void
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
pcb_t *removeChild(pcb_t *parent);

/**
*  @brief Remove a target child from any process.
*  @param pcb_t* child to seek and remove
*  @return pcb_t* the removed process
*  @warning Call this functiion before initPcbs() raise a PANIC()
*/
pcb_t *outChild(pcb_t *child);

#endif
