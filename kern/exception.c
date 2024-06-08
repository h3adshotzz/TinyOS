//===----------------------------------------------------------------------===//
//
//                                  tinyOS
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//	Copyright (C) 2024, Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

/**
 * Name:	exception.c
 * Desc:	Kernel Exception Handlers.
*/

#include <tinylibc/stdint.h>
#include <tinylibc/string.h>
#include <tinylibc/stddef.h>

#include <kern/defaults.h>
#include <kern/kprintf.h>
#include <kern/version.h>
#include <kern/vm/vm.h>
#include <arch/arch.h>
#include <kern/cpu.h>

#include <libkern/panic.h>

/* kernel handler annotations */
#define __KERNEL_FAULT_HANDLER
#define __KERNEL_ABORT_HANDLER

/* Types that will only be used here */
typedef vm_address_t	fault_address_t;
typedef int				fault_type_t;

/**
 * AArch64 First-stage Exception Handlers
*/
void arm64_handler_synchronous (arm64_exception_frame_t *);
void arm64_handler_serror (arm64_exception_frame_t *);
void arm64_handler_fiq (arm64_exception_frame_t *);
void arm64_handler_irq (arm64_exception_frame_t *);

/**
 * Second-stage Exception Handlers
*/
static void handle_breakpoint (arm64_exception_frame_t *);
static void handle_svc (arm64_exception_frame_t *);

/**
 * The Abort inspector determines the type of Abort that has occured, and the
 * handler is then dispatched to deal with the exception. From the First-stage
 * exception handler, we just call handle_abort().
*/
typedef void (*abort_inspector_t)	(fault_status_t *, fault_type_t *, uint32_t);
typedef void (*abort_handler_t)		(arm64_exception_frame_t *, fault_status_t, fault_status_t);

/* Abort Inspectors */
static void inspect_data_abort (fault_status_t *, fault_type_t *, uint32_t);
static void inspect_instruction_abort (fault_status_t *, fault_type_t *, uint32_t);

/* Abort type handlers */
static void handle_data_abort (arm64_exception_frame_t *, fault_address_t, fault_status_t);
static void handle_instruction_abort (arm64_exception_frame_t *, fault_address_t, fault_status_t);
static void handle_prefetch_abort (arm64_exception_frame_t *, fault_address_t, fault_status_t);
static void handle_msr_trap (arm64_exception_frame_t *);

/* General Abort handler */
static void handle_abort (arm64_exception_frame_t *, abort_handler_t, abort_inspector_t);

/**
 * Panic message
*/
static void panic_with_thread_state (arm64_exception_frame_t *, vm_address_t, const char *, ...);

/**
 * Misc Helper Functions
*/
static inline int is_translation_vault (fault_status_t);
static inline int is_permission_vault (fault_status_t);
static inline int is_alignment_vault (fault_status_t);
static inline int is_vm_vault (fault_status_t);

static inline int vm_fault_get_level (fault_status_t);


/*******************************************************************************
 * Misc Helper Functions
*******************************************************************************/

static inline int is_vm_fault (fault_status_t status)
{
	switch (status) {
		case FSC_TRANSLATION_FAULT_L0:
		case FSC_TRANSLATION_FAULT_L1:
		case FSC_TRANSLATION_FAULT_L2:
		case FSC_TRANSLATION_FAULT_L3:
		case FSC_ACCESS_FLAG_FAULT_L1:
		case FSC_ACCESS_FLAG_FAULT_L2:
		case FSC_ACCESS_FLAG_FAULT_L3:
		case FSC_PERMISSION_FAULT_L1:
		case FSC_PERMISSION_FAULT_L2:
		case FSC_PERMISSION_FAULT_L3:
			return 1;
		default:
			return 0;
	}
}

static inline int is_translation_fault (fault_status_t status)
{
	switch (status) {
		case FSC_TRANSLATION_FAULT_L0:
		case FSC_TRANSLATION_FAULT_L1:
		case FSC_TRANSLATION_FAULT_L2:
		case FSC_TRANSLATION_FAULT_L3:
			return 1;
		default:
			return 0;
	}
}

static inline int is_permission_fault (fault_status_t status)
{
	switch (status) {
		case FSC_PERMISSION_FAULT_L1:
		case FSC_PERMISSION_FAULT_L2:
		case FSC_PERMISSION_FAULT_L3:
			return 1;
		default:
			return 0;
	}
}

static inline int is_alignment_fault (fault_status_t status)
{
	return (FSC_ALIGNMENT_FAULT == status);
}


/**
 * Name:	vm_fault_get_level
 * Desc:	Return the Translation Level the given fault occured at, or -1 if
 * 			the given fault_status_t is not a Translation fault.
*/
static inline int vm_fault_get_level (fault_status_t status)
{
	switch (status) {
		/* Level 0 */
		case FSC_TRANSLATION_FAULT_L0:
			return 0;

		/* Level 1 */
		case FSC_TRANSLATION_FAULT_L1:
		case FSC_ACCESS_FLAG_FAULT_L1:
		case FSC_PERMISSION_FAULT_L1:
		case FSC_SYNC_EXT_ABORT_TT_L1:
		case FSC_SYNC_PARITY_TT_L1:
			return 1;

		/* Level 2 */
		case FSC_TRANSLATION_FAULT_L2:
		case FSC_ACCESS_FLAG_FAULT_L2:
		case FSC_PERMISSION_FAULT_L2:
		case FSC_SYNC_EXT_ABORT_TT_L2:
		case FSC_SYNC_PARITY_TT_L2:
			return 2;

		/* Level 3 */
		case FSC_TRANSLATION_FAULT_L3:
		case FSC_ACCESS_FLAG_FAULT_L3:
		case FSC_PERMISSION_FAULT_L3:
		case FSC_SYNC_EXT_ABORT_TT_L3:
		case FSC_SYNC_PARITY_TT_L3:
			return 3;

		/* Not a translation fault */
		default:
			return -1;
	}
}


/**
 * Name:	inspect_data_abort
 * Desc:	Determine the fault type. This was borrowed from XNU. Inspect the 
 * 			ISS value to determine the fault_code and fault_status, to determine
 * 			whether the abort was on a read or a write.
*/
static void inspect_data_abort (fault_status_t *fault_code, 
		fault_type_t *fault_type, uint32_t iss)
{
	*fault_code = ISS_DA_FSC(iss);

	if ((iss & ISS_DA_WNR) && (!(iss & ISS_DA_CM) || is_permission_fault(*fault_code))) {
		*fault_type = (VM_PROT_READ | VM_PROT_WRITE);
	} else {
		*fault_type = VM_PROT_READ;
	}
}


/*******************************************************************************
 * Abort Handlers
*******************************************************************************/

/**
 * TODO: Think of how this will need to change when userland is added. When a
 * userland process triggers an exception we don't want to halt the CPU, only a
 * kernel (kernel_task) exception should do that.
 * 
 * At the moment, i'm leaning towards checking kernel/user land within the
 * "handle_data_abort", rather than the exception handler further down (which is
 * how XNU does it).
 * 
 * The abort handlers shouldn't halt the CPU. Halting the CPU should be reserved
 * for the first-stage exception handlers. 
 * 
*/

__KERNEL_ABORT_HANDLER
void handle_data_abort (arm64_exception_frame_t *frame, 
		fault_address_t fault_address, fault_status_t fault_status)
{

	/**
	 * Panic with a virtual memory Translation Fault, and fetch the level at
	 * which the fault occured.
	*/
	if (is_translation_fault (fault_status)) {
		/** TOOD: There is more handling to do here then just printing the fault type */
		panic_with_thread_state (frame, fault_address, "Data Abort - Translation Fault Level %d", 
			vm_fault_get_level (fault_status));
		return;
	}

	/**
	 * Panic with a virtual memory Permission Fault, and fetch the level at
	 * which the fault occured.
	*/
	if (is_permission_fault (fault_status)) {
		panic_with_thread_state (frame, fault_address, "Data Abort - Permissions Fault, Level %d",
			vm_fault_get_level (fault_status));
		return;
	}

	/**
	 * Panic with an Alignment Fault.
	*/
	if (is_alignment_fault (fault_status)) {
		panic_with_thread_state (frame, fault_address, "Alignment Fault");
		return;
	}

	panic_with_thread_state (frame, frame->far, "Data Abort - Unknown");
}


/*******************************************************************************
 * Kernel Panic Logging
*******************************************************************************/

/**
 * TODO: Rework the panic print function. There should be a basic "panic()" that
 * 			can be called with a single message.
*/

static void _print_panic_header (cpu_number_t cpu, uint32_t pid, 
		vm_address_t addr)
{
	kprintf ("\n---- KERNEL PANIC ----\n");
	kprintf ("CPU: %d: PID: %d: Kernel Panic at 0x%016llx: ",
		cpu, pid, addr);
}

static void _print_panic_os_info ()
{
	/**
	 * Output system information for future reference.
	*/
	kprintf ("TinyOS Version:\n%s\n\n", KERNEL_BUILD_VERSION);
	kprintf ("Kernel Version:\ntinyOS Kernel Version %s; %s; %s:%s/%s_%s\n\n",
		KERNEL_BUILD_VERSION, __TIMESTAMP__,
		DEFAULTS_KERNEL_BUILD_MACHINE,
		KERNEL_SOURCE_VERSION, KERNEL_BUILD_STYLE, KERNEL_BUILD_TARGET);

	kprintf ("Kernel Slide: 0x%016llx\n", 0x0);
	kprintf ("Kernel Base: 0x%016llx\n", 0x0);
	kprintf ("Machine: %s\n\n", "unk");

	kprintf ("---- End Panic ----\n");
}

static void _print_cpu_register_state (arm64_exception_frame_t *frame)
{
	exception_level_t cur_el;

	/**
	 * Output the saved CPU state from when the exception was generated. We have
	 * to output the registers 8 at a time, otherwise some wizzard shit occurs
	 * and values are shifted left 4-bytes.
	*/
	kprintf (
		"CPU State:\n"
		"  x0: 0x%016llx   x1: 0x%016llx   x2: 0x%016llx   x3: 0x%016llx\n"
		"  x4: 0x%016llx   x5: 0x%016llx   x6: 0x%016llx   x7: 0x%016llx\n",
		frame->regs[0], frame->regs[1], frame->regs[2], frame->regs[3],
		frame->regs[4], frame->regs[5], frame->regs[6], frame->regs[7]
	);

	kprintf (
		"  x8: 0x%016llx   x9: 0x%016llx  x10: 0x%016llx  x11: 0x%016llx\n"
		" x12: 0x%016llx  x13: 0x%016llx  x14: 0x%016llx  x15: 0x%016llx\n",
		frame->regs[8], frame->regs[9], frame->regs[10], frame->regs[11],
		frame->regs[12], frame->regs[13], frame->regs[14], frame->regs[15]
	);

	kprintf (
		" x16: 0x%016llx  x17: 0x%016llx  x18: 0x%016llx  x19: 0x%016llx\n"
		" x20: 0x%016llx  x21: 0x%016llx  x22: 0x%016llx  x23: 0x%016llx\n",
		frame->regs[16], frame->regs[17], frame->regs[18], frame->regs[19],
		frame->regs[20], frame->regs[21], frame->regs[22], frame->regs[23]
	);

	kprintf (
		" x24: 0x%016llx  x25: 0x%016llx  x26: 0x%016llx  x27: 0x%016llx\n"
		" x28: 0x%016llx   fp: 0x%016llx   lr: 0x%016llx   sp: 0x%016llx\n",
		frame->regs[24], frame->regs[25], frame->regs[26], frame->regs[27],
		frame->regs[28], frame->fp, frame->lr, frame->sp
	);

	kprintf ("  pc: 0x%016llx\n\n", frame->pc);

	/**
	 * Get the exception level at the time of the commit, and print the FAR and
	 * ESR registers.
	*/
	cur_el = 1;
	kprintf ("Exception taken at EL%d\n", cur_el);
	kprintf ("  FAR_EL%d: 0x%016llx\n", cur_el, frame->far);
	kprintf ("  ESR_EL%d: 0x%016llx\n\n", cur_el, frame->esr);
}

static void _print_backtrace (cpu_number_t cpu_num)
{
	/**
	 * Output the CPU backtrace.
	*/
	kprintf ("Backtrace (CPU%d):\n\n", cpu_num);
	kprintf ("Process name (0x%016llx): %s\n\n", 0x0, "test_process");
}

/**
 * Name:	panic_with_thread_state
 * Desc:	Print the Kernel Panic with the CPU register state, including the
 * 			ESR and FAR registers.
*/
static void panic_with_thread_state (arm64_exception_frame_t *frame, vm_address_t fault_address, const char *fmt, ...)
{
	exception_level_t	cur_el;
	cpu_number_t		cpu_num;
	cpu_t				fault_cpu;
	va_list				args;

	fault_cpu = cpu_get_current ();
	cpu_num = fault_cpu.cpu_num;

	_print_panic_header (cpu_num, 0, frame->pc);
	va_start (args, fmt);
	vprintf (fmt, args);
	va_end (args);
	kprintf ("\n\n");

	_print_cpu_register_state (frame);
	_print_backtrace (cpu_num);
	_print_panic_os_info ();
}

/**
 * Name:	panic
 * Desc:	Print the Kernel Panic without the CPU register state.
*/
static void panic2 (vm_address_t fault_address, const char *fmt, ...)
{
	exception_level_t	cur_el;
	cpu_number_t		cpu_num;
	cpu_t				fault_cpu;

	fault_cpu = cpu_get_current ();
	cpu_num = fault_cpu.cpu_num;

	_print_panic_header (cpu_num, 0, fault_address);
	kprintf ("%s\n\n", fmt);

	_print_panic_os_info ();
}

/*******************************************************************************
 * Second-stage Exception Handling
*******************************************************************************/

/**
 * Name:	handle_breakpoint
 * Desc:	Handle a Breakpoint Exception.
*/
__KERNEL_FAULT_HANDLER
void handle_breakpoint (arm64_exception_frame_t *frame)
{
	panic_with_thread_state (frame, frame->pc, "Breakpoint 64");
}

/**
 * Name:	handle_svc
 * Desc:	Handle a Supervisor Call Exception.
*/
__KERNEL_FAULT_HANDLER
void handle_svc (arm64_exception_frame_t *frame)
{
	panic2 (frame->pc, "Supervisor Call (64)");
}

/**
 * Name:	handle_abort
 * Desc:	Handle an Abort exception (Data, Instruction, Prefetch)
*/
__KERNEL_FAULT_HANDLER
void handle_abort (arm64_exception_frame_t *frame, abort_handler_t handler, abort_inspector_t inspect)
{
	fault_address_t		fault_address;
	fault_status_t		fault_code;
	fault_type_t		fault_type;

	/* Inspect the fault, and then call the handler */
	inspect(&fault_code, &fault_type, ESR_ISS(frame->esr));
	handler (frame, fault_address, fault_code);
}

/**
 * Name:	handle_msr_trap
 * Desc:	Handle a trapped MSR, MRS or System instruction.
*/
__KERNEL_ABORT_HANDLER
void handle_msr_trap (arm64_exception_frame_t *frame)
{
	panic2 (frame->pc, "Trapped MSR, MRS, or System instruction");
}

/*******************************************************************************
 * First-stage Exception Handling
*******************************************************************************/

void arm64_handler_synchronous (arm64_exception_frame_t *frame)
{
	esr_exception_class_t	class;
	//thread_t				thread;

	/* obtain the exception class from the ESR register */
	class = ESR_EC (frame->esr);

	/* handle each of the exception classes */
	switch (class) {
		/* Program Counter Alignment Fault */
		case ESR_EC_PC_ALIGN:
			/* tmp */
			panic_with_thread_state (frame, frame->far, "PC Alignment Fault");
			cpu_halt ();
			break;

		/* Data Abort (EL0 and EL1) */
		case ESR_EC_DABORT_EL0:
		case ESR_EC_DABORT_EL1:
			handle_abort (frame, (abort_handler_t) handle_data_abort, inspect_data_abort);
			cpu_halt ();
			break;

		/* Breakpoint */
		case ESR_EC_BRK_AARCH64:
			handle_breakpoint (frame);
			cpu_halt ();
			break;

		/* Supervisor Call */
		case ESR_EC_SVC_64:
			handle_svc (frame);
			break;

		/* MSR Trap */
		case ESR_EC_MSR_TRAP:
			handle_msr_trap (frame);
			cpu_halt ();
			break;

		/* Unknown Exception*/
		default:
			panic_with_thread_state (frame, frame->far, "Unknown Exception");
			cpu_halt ();
			break;
	}
}

void arm64_handler_serror (arm64_exception_frame_t *frame)
{
	kprintf ("arm64_handler_serror\n");
}

void arm64_handler_fiq (arm64_exception_frame_t *frame)
{
	uint32_t intid = arm64_read_icc_iar1_el1 ();
	arm64_write_icc_eoir1_el1 (intid);

	kprintf ("arm64_handler_fiq: intid: %d\n", intid);
}

int irq_count = 0;

void arm64_handler_irq (arm64_exception_frame_t *frame)
{
	uint32_t intid = arm64_read_icc_iar1_el1 ();
	arm64_write_icc_eoir1_el1 (intid);

	kprintf ("arm64_handler_irq(%d): intid: %d\n", irq_count, intid);

//	if (intid == 30)
//		arm64_timer_reset (0x5000000);

	irq_count++;
}
