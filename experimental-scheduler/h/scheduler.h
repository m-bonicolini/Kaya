extern void set_Timer();
extern pcb_t* get_current();
extern void inc_round();
extern void set_round_zero();
extern void reset_round();
extern state_t* get_sched();
extern unsigned int get_proc_count();
extern unsigned int get_sofblock_count();
extern void inc_proc_count();
extern void inc_softblock_count();
extern void dec_proc_count();
extern void dec_softblock_count();
extern unsigned int equal_round_priority(pcb_t* proc);
extern void set_current_NULL();
extern void wait_state();
extern void add_ready_queue(pcb_t *proc);
extern pcb_t* remove_ready_queue();
extern pcb_t* out_ready_queue(pcb_t *proc);
extern void insert_swap_queue(pcb_t *proc);
extern pcb_t* remove_swap_queue();
extern pcb_t* out_swap_queue(pcb_t *proc);
extern void scheduler();
extern void activate_scheduler(pcb_t * first_proc);









