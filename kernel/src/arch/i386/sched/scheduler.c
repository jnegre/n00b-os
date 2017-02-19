#include <kernel/sched.h>
#include <kernel/mm.h>
#include <kernel/panic.h>
#include <arch/i386/pit.h>
#include <arch/i386/pic.h>
#include <arch/i386/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

void irq0(void); // defined in irq0.S

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
	process_control_block_t* pcb;
	machine_state_t* ms;
} task_t;



//FIXME not SMP friendly. Would current_process_control_block() work in user mode here?
static task_t* current_task;

/* 
 * Returns the next gdt_info.
 * All parameters are used in (current task) and out (next task).
 */
uintptr_t irq0_C(uint16_t ss, uint32_t esp) {
	// update current task
	current_task->ms->ss = ss;
	current_task->ms->esp = esp;
	// switch to the next (valid) task
	current_task = current_task->next;
	// set out values
	ss = current_task->ms->ss;
	esp = current_task->ms->esp;
	return current_task->pcb->mm_info->gdt_info;
}

static void put_in_schedule_ring(task_t* task) {
	itr_disable();
	task->next = current_task->next;
	current_task->next = task;
	itr_enable();
}

void sched_new_thread(void (*run)(void*), void* data) {
	uint32_t tid = nexttid();

	//create the new pcb
	process_control_block_t* new_pcb = malloc(sizeof(process_control_block_t));
	if(new_pcb == NULL) {
		panic("Failed to allocate new pcb");
	}
	new_pcb->tid = tid;
	//we have the same mm_info & tgid than the current one
	process_control_block_t* current_pcb = current_process_control_block();
	new_pcb->tgid = current_pcb->tgid;
	new_pcb->mm_info = current_pcb->mm_info;

	// create a new stack
	// FIXME but not like this
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

	*(uint32_t*)(sp-=4) = (uint32_t)data;
	*(uint32_t*)(sp-=4) = 0xBAD1BAD1; // address that will be used by the ret at the end of the call
	*(uint32_t*)(sp-=4) = 0x00200286; //EFlags
	*(uint32_t*)(sp-=4) = cs; //CS
	*(uint32_t*)(sp-=4) = (uint32_t)run; //EIP
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
	new_task->ms = ms;
	new_task->pcb = new_pcb;

	put_in_schedule_ring(new_task);
}

void sched_setup_tick(void) {
	printf("Setup 1st task\n");
	static machine_state_t ms = {0,0};
	static task_t first_task = {
		.ms = &ms,
		.next = &first_task
	};
	first_task.pcb = current_process_control_block();

	current_task = &first_task;

	printf("Setting IRQ0 handler\n");
	itr_set_handler(32, 0x8E, 8, &irq0); //TODO check it's the correct type
	printf("Programming PIC\n");
	pit_configure(CHANNEL_0, SQUARE_WAVE_GENERATOR, PIT_FREQUENCY_HZ/1000); // -> 1000 times per second
	printf("Enabling IRQ 0\n");
	pic_enable(0);
}