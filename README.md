# KAYA OPERATING SYSTEM

KAYA is a monolithic kernel written for MIPS that is a Dijkstra  OS remake (the key system call are P() and V() ) with a round robin FIFO scheduler. 
It implement the basic handler of a modern OS and it can be used in the emebedded environment. 
It is written for the Operating System Course at University of Bologna and it run in the umps emulator. 
The project is a Free and Open Source Software under the GPL V2 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).

# Directory Structure

  - AUTHORS list the authors of the project
  - LICENSE is the text of the GPL V2
  - Makefile is the makefile of the project for the compilation in the umps environment
  - doc/ contain the static documentation of the project (KAYA specification document and umps emulator guide) as pdf. 
  
 It is possible to generate the html docs via the doxygen command. See Doxyfile for info about the documentation format
  - e/ static header
  - h/ dynamic header usable out of kernel space
  - src/ the source code of the kernel
  - umps/ header for the umps emulator
  - experimental-scheduler/ the dir with the experimental scheduler implementation

# Overview
The initial.c  init the kernel data struct and start the first process with the load of p2test.0.1.c test function. 
The file implement some multithread test with the basic KAYA systemcall described in the "Student Guide to the Kaya Operating System" located in the doc path. 
The round robin FIFO scheduler is implemented in src/scheduler.c and it is implemented as API for other kernel module. 
The file src/interrupts.c implement the interrupt handler and the file src/exceptions.c implement the exception handler and the system call handler. 
The kernel use the information hiding concept for interface and it is better to see the documentation of the header and  the implementation the API usage. 
The process stack start at RAMTOP-40960. The area is reserved for handler/kernel code. 
In future may be nice to implement a strategy for a dynamic relocation of the kernel code in the stack. 


### Note 
The experimental scheduler path is an alternative implementation of the scheduler with a system call refactoring and a new API. It include also a kutil module for utility functions(like memcpy(), memset()...). 
The experimental scheduler implement the priority queue and for now it work with some issue. 
 
For info mail to matteo.bonicolini@gmail.com
