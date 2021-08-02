/* SPDX-License-Identifier: GPL-2.0 */
/**
 *  @brief Header for the init of the handler
 *  
 *  This file expose only one function that allocate the handlers
 *  in the stack. The real implementation is made via HIDDEN function. 
 *
 * 
 *  @see exceptions.c for info about the real implementation 
 *  @copyright  GNU General Public License, version 2 
 *  @file
 *
 **/


/**
  * @brief  Init the exception handler and put the code in the stack
  * @param  void
  * @return void
  */
void init_exceptions(void);
