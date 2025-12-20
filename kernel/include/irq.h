#ifndef __IRQ_H__
#define __IRQ_H__

#include "common.h"

/* TODO: The decleration order of the members in the `TrapFrame'
 * structure below is wrong. Please re-orgainize it for the C
 * code to use the trap frame correctly.
 */

typedef struct TrapFrame {
	/*
	 * Stack layout at the moment of `call irq_handle` in do_irq.S:
	 *	pushal saves: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI (in this order),
	 *	so the current %esp points to EDI.
	 *	Below them are: irq, error_code, eip, cs, eflags (pushed by CPU/vec stub).
	 */
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	int32_t irq;
	uint32_t error_code;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
} TrapFrame;

#endif
