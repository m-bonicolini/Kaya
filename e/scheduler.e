/* SPDX-License-Identifier: GPL-2.0 */
/**
 *  @brief Interface for the scheduler API
 *  
 *  The file define the scheduler API. The scheduler is a round robin FIFO. 
 *  The state_t and the pcb_t are not defined in the
 *  header interface because they can be implemented in many way. The 
 *  set up to the real type is defined at compile time.
 *  The header define only functions that the caller can use in kernel 
 *  space and the API should not be used in user space programming. 
 *  There are no constraint about inclusion of any external file.
 *   
 *  @note Extend the current information hiding for this module 
 *  is needed. One thing to do is to add some handling function to 
 *  the module an easy handling of the current running process and 
 *  the scheduler. Also a Priority Queue API may be implemented 
 *   
 *  @see scheduler.c for info about the implementation and the logic 
 *  about it
 *  @copyright  GNU General Public License, version 2 
 *  @file
 *
 **/



/**
*  @brief A pointer to the running process.
*  @note This pointer will be removed because it is better to handle 
*  it with a specific function 
*/
pcb_t *running;


/**
  * @brief Make the copy of two processor state
  * @param state_t* source to copy;
  * @param state_t* destination of the copy;
  * @return void
  * @note Future version shall implement a memcopy in a kutil module.
  */
void state_t_copy (state_t* src, state_t* dst);


/**
  * @brief Update the current process time fields
  * @param int time to take into account
  * @return void
  */
void time(int now);

/**
*  @brief Scheduler control function for other modules
*  @param int command for the scheduler. One of CONTEXTSWITCH, SYSSCHED, INTSCHED
*  @param int boolean to take time update of the process via time(). 
*  If set to TRUE time() is called. Otherwise no 
*  @return void
*/
void extern_scheduler(int command, int interruptFlag);

/**
  * @brief Ininitialize the module and insert the first proc 
  * in the scheduler
  * @param pcb_t* the first process to insert in the scheduler;
  * @return void
  * @warning Call this function more than once raise a PANIC()
  */
void init_Sched(pcb_t *first_proc);

