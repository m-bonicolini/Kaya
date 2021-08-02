extern void create_process();

extern void terminate_process(pcb_t *proc);

extern pcb_t* verhogen(int *sem);

extern void passern(int *sem);

extern void spec_exc_vector(unsigned int type, state_t* old_state, state_t* new_state); 

extern void get_cputime(cpu_t now);

extern void waitclock();

extern /*int*/void wait_io(unsigned int intline, unsigned int devnum, unsigned int flag);








