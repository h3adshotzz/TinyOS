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
 * Name:	machine_irq.h
 * Desc:	Kernel Machine IRQ Interface. Abstracts the underlying interrupt
 * 			controller driver code allowing for interrupt controllers other than
 * 			GICv3.
 * 
 * 			NOTE: For now, this only supports GICv3. Abstracting the interrupt
 * 			controller is the end-goal.
*/

#ifndef __MACHINE_IRQ_H__
#define __MACHINE_IRQ_H__

#include <tinylibc/limits.h>
#include <tinylibc/stdint.h>

typedef uint32_t		intid_t;

struct irq_data {
	intid_t		irq;
	void		*data;		/* chip-specific data, i.e. GICv3 */
};

struct irq_interface {

	const char	*name;
	void		(*irq_init)(struct irq_data *);

	void		(*irq_enable)(struct irq_data *);
	void		(*irq_disable)(struct irq_data *);

	void		(*irq_ack)(struct irq_data *);
	void		(*irq_eoi)(struct irq_data *);


};

/* GICv3-specific */
kern_return_t
gic_interface_init (vm_address_t dist_base, vm_address_t redist_base);


//kern_return_t machine_configure_interrupts ();
//kern_return_t machine_enable_interrupts ();
//kern_return_t machine_disable_interrupts ();


#endif /* __machine_irq_h__ */