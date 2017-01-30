#ifndef _KERNEL_SCHED_H
#define _KERNEL_SCHED_H

#include <kernel/mm.h>

typedef struct process_control_block {
	mm_info_t* mm_info;
} process_control_block_t;

process_control_block_t* current_process_control_block(void);

void sched_init_process_control_block(void);

#endif