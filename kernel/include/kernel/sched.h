#ifndef _KERNEL_SCHED_H
#define _KERNEL_SCHED_H

#include <kernel/mm.h>

typedef struct process_control_block {
	uint32_t tgid; // thread group id aka pid
	uint32_t tid; // thread id
	mm_info_t* mm_info;
} process_control_block_t;

process_control_block_t** current_process_control_block_ptr(void);
process_control_block_t* current_process_control_block(void);

/*
 * Creates a new thread for the process.
 * run: function to be executed by the new thread.
 * data: parameter for the function "run"
 * The function "run"" should never return, or bad things will happen.
 */
 //TODO function should be able to finish + exit code
void sched_new_thread(void (*run)(void*), void* data);

/*
 * Puts the current thread to sleep for a specific duration.
 * ms: the duration in ms
 */
void sched_sleep(uint32_t ms);

/*
 * Provides a hint to the implementation to reschedule the execution of threads,
 * allowing other threads to run.
 */
void sched_yield(void);

void sched_init_process_control_block(void);
void sched_setup_tick(void); //FIXME rename

#endif