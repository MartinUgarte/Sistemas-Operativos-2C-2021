/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



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
TRAPHANDLER_NOEC(trap_0,T_DIVIDE) //nombre:trap_n, trap numbers
TRAPHANDLER_NOEC(trap_1,T_DEBUG)
TRAPHANDLER_NOEC(trap_2,T_NMI)
TRAPHANDLER_NOEC(trap_3,T_BRKPT)
TRAPHANDLER_NOEC(trap_4,T_OFLOW)
TRAPHANDLER_NOEC(trap_5,T_BOUND)
TRAPHANDLER_NOEC(trap_6,T_ILLOP)
TRAPHANDLER_NOEC(trap_7,T_DEVICE)
TRAPHANDLER(trap_8,T_DBLFLT)
TRAPHANDLER(trap_10,T_TSS)
TRAPHANDLER(trap_11,T_SEGNP)
TRAPHANDLER(trap_12,T_STACK)
TRAPHANDLER(trap_13,T_GPFLT)
TRAPHANDLER(trap_14,T_PGFLT)
TRAPHANDLER_NOEC(trap_16,T_FPERR)
TRAPHANDLER(trap_17,T_ALIGN)
TRAPHANDLER_NOEC(trap_18,T_MCHK)
TRAPHANDLER_NOEC(trap_19,T_SIMDERR)
TRAPHANDLER_NOEC(trap_48, T_SYSCALL)


/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
    pushl %ds          //push del data segment
    pushl %es          //push del extra segment
    pushal             //push de todos los registros en el orden de PushRegs (trap.h)
    mov $GD_KD, %ax    //carga GD_KD en el registro auxiliar ax
    mov %ax, %ds       //carga GD_KD en el data segment
    mov %ax, %es       //carga GD_KD en extra segment
    pushl %esp         //push del puntero al stack para que sea el parametro de la funcion trap
    call trap          //llamo a la funcion trap

