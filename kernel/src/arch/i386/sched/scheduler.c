#include <kernel/sched.h>
#include <kernel/mm.h>
#include <kernel/panic.h>
#include <arch/i386/pit.h>
#include <arch/i386/pic.h>
#include <arch/i386/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void irq0_handler(void); // defined in interrupts.S
void int50_handler(void); // defined in interrupts.S
void int51_handler(void); // defined in interrupts.S

static uint32_t nexttid(void) {
	//FIXME quick'n'dirty
	static uint32_t next = 1;
	itr_disable();
	uint32_t result = ++next;
	itr_enable();
	return result;
}

/* most of the machine state is stored in the pointed stack */
typedef struct machine_state {
	uint16_t ss;
	uint32_t esp;
} machine_state_t;

typedef struct task {
	struct task* next;
	struct task* previous;
	uint32_t remaining_ticks;
	process_control_block_t* pcb;
	machine_state_t* ms;
} task_t;



//FIXME not SMP friendly. Would current_process_control_block() work in user mode here?
/* The task currently running. Changed at the last second in sched_switch_next_task. If NULL, we're in idle. */
static task_t* current_task;
static bool switching;
/* The task we're switching to. If NULL, we're going in idle mode. */
static task_t* switched_task = NULL;
/* Pointed list of sleeping tasks */
static task_t* sleeping_task = NULL;
/* Tasks to kill asap (next tick) */
static task_t* doomed_task = NULL;
/* The idle task, does nothing but hlt */
static task_t* idle_task; //TODO give it PID 0

/* 
 * Returns the next gdt_info.
 * All parameters are used in (current task) and out (next task).
 */
uintptr_t sched_switch_next_task(uint16_t ss, uint32_t esp) {
	if(switching) {
		if(current_task != NULL) {
			// update current task
			current_task->ms->ss = ss;
			current_task->ms->esp = esp;
		} else {
			// update idle task
			idle_task->ms->ss = ss;
			idle_task->ms->esp = esp;
		}

		if(switched_task != NULL) {
			// switch to the next (valid) task
			current_task = switched_task;
			current_task->remaining_ticks = current_task->pcb->priority;
			// set out values
			ss = current_task->ms->ss;
			esp = current_task->ms->esp;
		} else {
			// idle task
			current_task = NULL;
			ss = idle_task->ms->ss;
			esp = idle_task->ms->esp;
		}
	}
	return (current_task!=NULL ? current_task : idle_task)->pcb->mm_info->gdt_info;
}

/*
 * Do not call when interrupts are enabled.
 */
static void put_in_schedule_ring(task_t* task) {
	if(current_task != NULL) {
		// TODO insert BEFORE current_task instead of after
		task->next = current_task->next;
		task->previous = current_task;
		task->next->previous = task;
		current_task->next = task;
	} else if(switched_task != NULL) {
		task->next = switched_task->next;
		task->previous = switched_task;
		task->next->previous = task;
		switched_task->next = task;
	} else {
		task->next = task;
		task->previous = task;
		switched_task = task;
		switching = true;
	}
}

/* Called every 1 ms */
void sched_ms_tick(void) {
	// wake up sleeping tasks
	if(sleeping_task != NULL) {
		-- sleeping_task->remaining_ticks;
		while(sleeping_task != NULL && sleeping_task->remaining_ticks == 0) {
			task_t* next_sleeping = sleeping_task->next;
			put_in_schedule_ring(sleeping_task);
			sleeping_task = next_sleeping;
		}
	}
	if(current_task != NULL) {
		if(-- current_task->remaining_ticks == 0) {
			switched_task = current_task->next;
			switching = true;
		} else {
			switching = false;
		}
	}
}


/* Removes the current task from the schedule ring and sets switched_task.
 * Do not call when interrupts are enabled.
 */
static task_t* remove_from_schedule_ring() {
	task_t* removed = current_task;
	if(current_task == current_task->next) {
		// home alone
		switched_task = NULL;
	} else {
		//TODO what a mess
		removed->previous->next = removed->next;
		removed->next->previous = removed->previous;
		switched_task = removed->next;
	}
	return removed;
}

void sched_park_task(uint32_t ms) {
	switching = true;
	// Note: the next task will only get what's left of the current ms, not a full one
	if(ms == 0) {
		switched_task = current_task->next;
	} else {
		task_t* going_to_bed = remove_from_schedule_ring();
		if(sleeping_task == NULL) {
			going_to_bed->next = NULL;
			going_to_bed->remaining_ticks = ms;
			sleeping_task = going_to_bed;
		} else {
			uint32_t remaining = ms;
			task_t* previous = NULL;
			task_t* t;
			for(t = sleeping_task; t!=NULL && t->remaining_ticks < remaining; t = t->next) {
				remaining -= t->remaining_ticks;
				previous = t;
			}
			// insert between previous and t
			going_to_bed->remaining_ticks = remaining;
			going_to_bed->next = t;
			if(previous != NULL) {
				previous->next = going_to_bed;
			} else {
				sleeping_task = going_to_bed;
			}
			// remove remaining ms from the next sleeping task
			if(t!=NULL) {
				t->remaining_ticks -= remaining;
			}
		}
	}
}

void sched_yield(void) {
	sched_sleep(0);
}

static task_t* create_new_task(int (*func)(void*), void* data, enum thread_priority priority) {
	uint32_t tid = nexttid();

	//create the new pcb
	process_control_block_t* new_pcb = malloc(sizeof(process_control_block_t));
	if(new_pcb == NULL) {
		panic("Failed to allocate new pcb");
	}
	new_pcb->tid = tid;
	new_pcb->priority = priority;
	//we have the same mm_info & tgid than the current one
	process_control_block_t* current_pcb = current_process_control_block();
	new_pcb->tgid = current_pcb->tgid;
	new_pcb->mm_info = current_pcb->mm_info;

	// create a new stack
	// TODO but not like this
	uintptr_t stack_top = 0xFFBFC000 - (tid-2) * 4*4096;
	for(int i=1; i<=4; i++) {
		uintptr_t page = mm_alloc_physical_page(true);
		if(page == 0) {
			panic("Failed to allocate a page to create the stack");
		}
		if(mm_map_page(page, stack_top - 0x1000*i)) {
			panic("Can't map new page for the stack");
		}
	}
	// add the canary at the top
	uintptr_t base = stack_top - 4*4096;
	*(uint32_t*)(base | 0xC) = 0xDEADBEEF;
	// and bottom
	*(uint32_t*)(base | 0x3FF0) = 0xDEADBEEF;
	// pcb
	*(process_control_block_t**)(base | 0x3FF8) = new_pcb;

	//fill the stack with all the "saved" registers for the various pops & (i)ret
	uint32_t cs = 0x08;
	uint32_t ds = 0x10;
	uintptr_t sp = base | 0x3FF0;

	*(uint32_t*)(sp-=4) = (uint32_t)42; // dummy param for sched_exit //TODO not a dummy param
	*(uint32_t*)(sp-=4) = (uint32_t)data; // param for run
	*(uint32_t*)(sp-=4) = (uint32_t)&sched_exit; // address that will be used by the ret at the end of the call of run
	*(uint32_t*)(sp-=4) = 0x00200286; //EFlags
	*(uint32_t*)(sp-=4) = cs; //CS
	*(uint32_t*)(sp-=4) = (uint32_t)func; //EIP
	*(uint32_t*)(sp-=4) = 0; // EAX
	*(uint32_t*)(sp-=4) = 0; // ECX
	*(uint32_t*)(sp-=4) = 0; // EDX
	*(uint32_t*)(sp-=4) = 0; // EBX
	*(uint32_t*)(sp-=4) = 0; // reserved
	*(uint32_t*)(sp-=4) = 0; // EBP
	*(uint32_t*)(sp-=4) = 0; // ESI
	*(uint32_t*)(sp-=4) = 0; // EDI
	*(uint32_t*)(sp-=4) = ds; // DS
	*(uint32_t*)(sp-=4) = ds; // ES
	*(uint32_t*)(sp-=4) = ds; // FS
	*(uint32_t*)(sp-=4) = ds; // GS

	//create the new machine_state
	machine_state_t* ms = malloc(sizeof(machine_state_t));
	if(ms == NULL) {
		panic("Failed to allocate new machine_state");
	}
	ms->ss = ds;
	ms->esp = sp;

	//create the new task
	task_t* new_task = malloc(sizeof(task_t));
	if(new_task == NULL) {
		panic("Failed to allocate new task");
	}
	new_task->remaining_ticks = priority;
	new_task->ms = ms;
	new_task->pcb = new_pcb;
	return new_task;
}

void sched_new_thread(uint32_t* tid, int (*func)(void*), void* data, enum thread_priority priority) {
	itr_disable();
	task_t* new_task = create_new_task(func, data, priority);
	put_in_schedule_ring(new_task);
	itr_enable();
	if(tid != NULL) {
		*tid = new_task->pcb->tid;
	}
}

void sched_kill_task(int res) {
	//TODO do something with res
	//printf("Kill %u with res %u\n",current_process_control_block()->tid , res);
	switching = true;
	task_t* dead = remove_from_schedule_ring();
	dead->next = doomed_task;
	doomed_task = dead;
	//FIXME clean-up the task in the next tick
	//FIXME clean-up the process if it's the last task of its tgid'
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static noreturn int idle(void* not_used) {
	#pragma GCC diagnostic pop
	while(true) {
		asm("hlt;");
	}
}

void sched_init_tasks(void) {
	// setup idle task
	idle_task = create_new_task(idle, NULL, PRIORITY_HIGH);

	// Setup 1st task
	task_t* first_task = malloc(sizeof(task_t));
	if(first_task == NULL) {
		panic("Failed to allocate 1st task");
	}
	machine_state_t* ms = malloc(sizeof(machine_state_t));
	if(ms == NULL) {
		panic("Failed to allocate 1st machine state");
	}
	first_task->ms = ms;
	first_task->next = first_task;
	first_task->pcb = current_process_control_block();
	first_task->remaining_ticks = first_task->pcb->priority;

	current_task = first_task;

	// Setting int 50 handler
	itr_set_handler(50, 0x8E, 8, &int50_handler); //TODO check it's the correct type
	// Setting int 51 handler
	itr_set_handler(51, 0x8E, 8, &int51_handler); //TODO check it's the correct type
	// Setting IRQ0 handler
	itr_set_handler(32, 0x8E, 8, &irq0_handler); //TODO check it's the correct type
	// Programming PIC
	pit_configure(CHANNEL_0, SQUARE_WAVE_GENERATOR, PIT_FREQUENCY_HZ/1000); // -> 1000 times per second
	// Enabling IRQ 0
	pic_enable(0);
	printf("Scheduler initialized\n");
}