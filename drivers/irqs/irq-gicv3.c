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
 * Name:	drivers/irqs/irq-gicv3.c
 * Desc:	ARM GICv3 Driver.
*/

#include <arch/arch.h>

#include <kern/vm/vm.h>
#include <kern/machine.h>
#include <kern/defaults.h>
#include <kern/machine/machine_irq.h>

#include <drivers/irqs/irq-gicv3.h>

struct gicv3_data {
	int				version;

	vm_address_t	dist_base;
	vm_address_t	redist_base;

	unsigned int	max_redist_idx;
	bool			initialised;

	struct gicv3_dist_frame		*dist;
	struct gicv3_redist_frame	*redist;

	uint32_t		flags;
};

struct gicv3_dist_frame		*__dist;
struct gicv3_redist_frame	*__redist;

/* GIC interface data */
PRIVATE_STATIC_DEFINE(struct gicv3_data) gic_data 
	__attribute__((section(".data")));

/**
 * Total number of SGIs. The SGI/PPI range is 0-31.
*/
#define SGI_NR		16

/* Minimum GIC version */
#define PLAT_GIC_MIN_VERSION	3

/* System register read/writes */
DEFINE_SYSREG_READ_FUNC(mpidr_el1)


/**
 * Name:	gic_get_redist_id
 * Desc:	Get the redistributor index for a given CPU affinity register value.
*/
static uint32_t
gic_get_redist_id (uint32_t affinity)
{
	uint32_t index = 0;
	do {
		if (gic_data.redist[index].lpis.typer[1] == affinity)
			return index;
		index++;
	} while (index <= gic_data.max_redist_idx);
	return 0xffffffff;
}

static int
gic_get_version ()
{
	int version;

	version = (gic_data.dist->pidr2 >> 4) & 0xf;
	if (version < PLAT_GIC_MIN_VERSION)
		gicv3_log ("GIC version mistmatch, current '%d', minimum: %d\n",
			version, PLAT_GIC_MIN_VERSION);

	return version;
}

/*******************************************************************************
 * GICv3 Configuration
*******************************************************************************/

/**
 * TODO:	Eventually these should be called via the irq_interface struct,
 * 			using the machine_irq interface
*/

void gic_dist_init ()
{
	/* Get GIC version */
	gic_data.version = gic_get_version ();

	/**
	 * Configure the Distributor Control Register. We need to enable Affinity
	 * Routing and Non-Secure Group 1 interrupts.
	*/
	gic_data.dist->ctlr = (GICD_CTLR_ARE_NS_BIT | GICD_CTLR_DS_BIT);
	gic_data.dist->ctlr |= GICD_CTLR_ENABLE_G1NS_BIT;

	dsbsy ();
	isb ();
}

void gic_redist_init ()
{
	cpu_number_t cpu_num;
	uint32_t redist_id;

	/**
	 * Configure the Redistributor for the currently executing CPU. This will
	 * need slight changes when SMP is implemented.
	*/
	cpu_num = machine_get_cpu_num ();
	gic_data.max_redist_idx = machine_get_max_cpu_num ();

	/**
	 * Obtain the redistributor index for the above MPIDR_EL1 value, and use it
	 * to configure the correct redistributor.
	*/
	redist_id = gic_get_redist_id (arm64_read_affinity ());
	gic_data.redist[redist_id].lpis.waker &= ~GICR_WAKER_PS_BIT;

	/**
	 * Poll GICR_WAKER.ChildrenAsleep until it reads 0, once it does the Redist
	 * has woken up.
	*/
	while (gic_data.redist[redist_id].lpis.waker & GICR_WAKER_CA_BIT)
		gicv3_log ("CPU%d: waiting for Redistributor to wake up\n", cpu_num);
	
	gicv3_log ("CPU%d: found redistributor '%d' region: 0x%llx\n",
		cpu_num, redist_id, &gic_data.redist[redist_id]);

	dsbsy ();
	isb ();
}

void gic_cpuif_init ()
{
	/**
	 * Configure the CPU Interface for the currently executing CPU.
	*/
	arm64_icc_sre_el1_write (arm64_icc_sre_el1_read () |
		(ICC_SRE_EN_BIT | ICC_SRE_SRE_BIT));

	arm64_icc_pmr_el1_write (0xff);
	arm64_icc_igrpen1_el1_write (arm64_icc_igrpen1_el1_read () | 0x1);

	dsbsy ();
	isb ();
}

kern_return_t
gic_interface_init (vm_address_t dist_base, vm_address_t redist_base)
{
	/* Set the base addresses */
	gic_data.dist_base = dist_base;
	gic_data.redist_base = redist_base;

	/* Initialise the memory-mapped register frames */
	gic_data.dist = (struct gic_dist_frame *) dist_base;
	gic_data.redist = (struct gic_redist_frame *) redist_base;

	/* Configure the Distributor */
	gic_dist_init ();

	/* Configure the Redistributor */
	gic_redist_init ();

	/* Configure the CPU Interface */
	gic_cpuif_init ();

	/* Mark the GIC as being initialised, and return */
	gic_data.initialised = 1;
	gicv3_log ("Interrupt controller configured: GICv%d\n", gic_data.version);

	return KERN_RETURN_SUCCESS;
}

#define GIC_IRQ_CONTROL_DISABLE		0
#define GIC_IRQ_CONTROL_ENABLE		1

static void
__gic_irq_control (uint32_t intid, int ctrl)
{
	if (intid < 31) {
		uint32_t redist_id, id;

		/* Grab the redistributor and check it's within range */
		redist_id = gic_get_redist_id (arm64_read_affinity ());
		if (redist_id > gic_data.max_redist_idx)
			return KERN_RETURN_FAIL;

		/* Figure out which bit within the registers to modify */
		id = 1 << (intid & 0x1f);

		/* Enable the interrupt */
		if (ctrl == GIC_IRQ_CONTROL_ENABLE)
			gic_data.redist[redist_id].sgis.isenabler[0] |= id;
		else if (ctrl == GIC_IRQ_CONTROL_DISABLE) {
			gic_data.redist[redist_id].sgis.icenabler[0] |= id;
		}

		dmbst ();
		isb ();
	}
}

/*******************************************************************************
 * GICv3 Interrupt Configuration
*******************************************************************************/

kern_return_t
gic_irq_register (uint32_t intid, uint32_t priority)
{
	/**
	 * The configuration of the interrupt depends on the type, i.e. SGI/PPI or
	 * SPI. SGI/PPI are configured on the Redistributor, whereas SPIs are 
	 * configured on the Distributor.
	*/
	if (intid < 31) {
		uint32_t redist_id, group, mod, id;

		/* Grab the redistributor and check it's within range */
		redist_id = gic_get_redist_id (arm64_read_affinity ());
		if (redist_id > gic_data.max_redist_idx)
			return KERN_RETURN_FAIL;

		/* Set the interrupts priority */
		gic_data.redist[redist_id].sgis.ipriorityr[intid] = priority;

		/* Figure out which bit within the registers to modify */
		id = 1 << (intid & 0x1f);

		/* Calculate which field within the following registers to modify */
		group = gic_data.redist[redist_id].sgis.igroupr[0];
		mod = gic_data.redist[redist_id].sgis.igrpmodr[0];

		/* Only Non-secure Group 1 are supported */
		group = (group | id);
		mod = (mod & ~id);

		/* Write the Group and Mod values back */
		gic_data.redist[redist_id].sgis.igroupr[0] = group;
		gic_data.redist[redist_id].sgis.igrpmodr[0] = mod;

		/* Enable the interrupt */
		__gic_irq_control (intid, GIC_IRQ_CONTROL_ENABLE);

		dsbsy ();
		isb ();

	} else if (intid < 1020) {
		gicv3_log ("INTID '%d' is not configurable\n", intid);
		return KERN_RETURN_FAIL;
	} else {
		gicv3_log ("Extended interrupt range not supported\n");
		return KERN_RETURN_FAIL;
	}

	gicv3_log ("configured interrupt '%d' with priority: 0x%x\n", 
		intid, priority);
	return KERN_RETURN_SUCCESS;
}

void
gic_irq_enable (uint32_t intid)
{
	__gic_irq_control (intid, GIC_IRQ_CONTROL_ENABLE);
}

void
gic_irq_disable (uint32_t intid)
{
	__gic_irq_control (intid, GIC_IRQ_CONTROL_DISABLE);
}

#define GIC_IRM_DISABLE		0
#define GIC_IRM_ENABLE		1


#define CREATE_SGIR_VALUE(_aff3, _aff2, _aff1, _intid, _irm, _tgt)	\
	((_aff3 << 48) | \
	(_irm << 40) | \
	(_aff2 << 32) | \
	(_intid << 24) | \
	(_aff1 << 16) | \
	(_tgt))

void
gic_send_sgi (uint32_t intid, uint32_t target)
{
	uint32_t aff3, aff2, aff1, mpidr, sgi_val;

	/* Grab affinity values */
	mpidr = arm64_read_mpidr_el1 ();
	aff1 = MPIDR_AFFLVL1_VAL(mpidr);
	aff2 = MPIDR_AFFLVL2_VAL(mpidr);
	aff3 = MPIDR_AFFLVL3_VAL(mpidr);

	sgi_val = CREATE_SGIR_VALUE (aff3, aff2, aff1, intid, GIC_IRM_DISABLE, target);
	gicv3_log ("Generating INTID '%d' for affinity %d.%d.%d.%d (SGI: 0x%llx)\n",
		intid, aff3, aff2, aff1, target, sgi_val);

	/* Write the SGI value to trigger the interrupt */
	arm64_write_icc_sgi1r_el1 (sgi_val);

	dsbsy ();
	isb ();
}



/*
	TODO:
	gic_irq_acknowledge
	gic_irq_eoi

*/

/* GICv3 Interface */
//static struct irq_interface gic_interface = {
//	.name		= 	"GICv3",
//	.irq_init	=	gic_interface_init,
//};
