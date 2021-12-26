/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(trap_divide_error, T_DIVIDE);// 0
TRAPHANDLER_NOEC(trap_debug_exception, T_DEBUG);// 1
TRAPHANDLER_NOEC(trap_nmi_interrupt, T_NMI);// 2
TRAPHANDLER_NOEC(trap_breakpoint, T_BRKPT);// 3
TRAPHANDLER_NOEC(trap_overflow, T_OFLOW);// 4

TRAPHANDLER_NOEC(trap_bound_range_exceeded, T_BOUND);// 5
TRAPHANDLER_NOEC(trap_invalid_opcode, T_ILLOP);// 6
TRAPHANDLER_NOEC(trap_device_not_available, T_DEVICE);// 7
TRAPHANDLER(trap_double_fault, T_DBLFLT);// 8
/* TRAPHANDLER_NOEC(trap_coprocessor_segment_overrun, T_COPROC);// 9 */ 

TRAPHANDLER(trap_invalid_tss, T_TSS); // 10
TRAPHANDLER(trap_segment_not_present, T_SEGNP);// 11
TRAPHANDLER(trap_stack_segment_fault, T_STACK);// 12
TRAPHANDLER(trap_general_protection, T_GPFLT);// 13
TRAPHANDLER(trap_page_fault, T_PGFLT);// 14

/* TRAPHANDLER_NOEC(trap_reserver, T_RES);// 15 */
TRAPHANDLER_NOEC(trap_fpu_floating_point_error, T_FPERR);// 16
TRAPHANDLER(trap_alignment_check, T_ALIGN);// 17
TRAPHANDLER_NOEC(trap_machine_check, T_MCHK);// 18
TRAPHANDLER_NOEC(trap_simd_floating_point_exception, T_SIMDERR);// 19

/* TRAPHANDLER_NOEC(trap_virtualization_exception, 20);// 20 */
/* TRAPHANDLER(trap_control_protection_exception, 21);// 21 */

TRAPHANDLER_NOEC(trap_syscall, T_SYSCALL);// 48
/* TRAPHANDLER_NOEC(trap_catchall, T_DEFAULT);// 500 */

TRAPHANDLER_NOEC(trap_irq_timer, IRQ_OFFSET+IRQ_TIMER);
TRAPHANDLER_NOEC(trap_irq_kdb, IRQ_OFFSET+IRQ_KBD);
TRAPHANDLER_NOEC(trap_irq_serial, IRQ_OFFSET+IRQ_SERIAL);

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	/* push registers */
	pushl %ds;
	pushl %es;
	pushal;

	/* Load GD_DK */
	mov $GD_KD, %ax;
	mov %ax, %ds;
	mov %ax, %es;

	/* Pass Trapframe pointer as param to trap */
	pushl %esp;

	call trap;
	