/* SPDX-License-Identifier: GPL-2.0 */

/**
 *
 *  @brief Interface header for the Active Semaphore List. 
 *  
 *  The file define the low level semaphore handling interface. 
 * 
 *  @see asl.c for info about the implementation
 *  @copyright GNU General Public License, version 2 
 *  @file
 *
 **/
#ifndef ASL_E
#define ASL_E
#include "pcb.e"
#include "../umps/const.h"
#include "../h/types10.h"
#include "../h/listx.h"


/**
*  @brief Insert a blocked process in an associated data struct (actually a FIFO)
*  @param S32*   a pointer to semaphore
*  @param pcb_t* a pointer to the  process to insert in the semaphore data struct
*  @return FALSE if the insertion is ok, TRUE otherwise
*  @note When called the result of the BlockCount() is incremented by 1
*  @warning Call this functiion before initSemd() raise a PANIC()
*/
int insertBlocked(S32 *semAdd, pcb_t *p);

/**
*  @brief Remove the first process that is blocked on semaphore (FIFO policy)
*  @param S32* the target semaphore
*  @return pcb_t* the first process in the queue, NULL if queue is empty
*  @note When called the result of the BlockCount() is decremented by 1
*  @warning Call this functiion before initSemd() raise a PANIC()
*/
pcb_t *removeBlocked(S32 *semAdd);

/**
*  @brief Remove a process from any allocated semaphore in the system
*  @param pcb_t*  the process to remove
*  @return pcb_t* the target process, NULL if process is not associated to any sempahore
*  @note When called the result of the BlockCount() is decremented by 1
*  @warning Call this functiion before initSemd() raise a PANIC() 
*/
pcb_t *outBlocked(pcb_t *p);

/**
*  @brief Get the first process associated to a semaphore
*  @param pcb_t* a pointer to the first process of the semAdd queue
*  @return pcb_t* the target process, NULL if process is not associated to any sempahore
*  @warning Call this functiion before initSemd() raise a PANIC() 
*/
pcb_t *headBlocked(S32 *semAdd);

/**
*  @brief Init the module. This function is to call only once before any function usage in the module.
*  It init the data struct of the module
*  @param void
*  @return void
*  @warning Call this function more than once raise a PANIC()
*/
void initSemd(void);

/**
*  @brief Count the number of blocked process in the system by this module
*  @param void
*  @return int the number of blocked process in any allocated semaphore
*  @warning Call this functiion before initSemd() raise a PANIC()
*/
int BlockCount(void);
#endif
