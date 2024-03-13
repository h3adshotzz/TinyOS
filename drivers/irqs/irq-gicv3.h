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
 * Name:	drivers/irqs/irq-gicv3.h
 * Desc:	ARM GICv3 Driver.
*/

#ifndef __DRIVER_IRQ_GICV3_H__
#define __DRIVER_IRQ_GICV3_H__

#include <tinylibc/limits.h>
#include <tinylibc/stdint.h>

/* GICv3 log */
#define gicv3_log(fmt, ...)		interface_log ("GICv3", fmt, ##__VA_ARGS__)

/*******************************************************************************
 * GICv3 Distributor Registers and Bit Definitions
 ******************************************************************************/

/**
 * GICD_CTLR, Distributor Control Register Bits
*/
#define GICD_CTLR_ENABLE_G0_BIT		BIT_32(0)
#define GICD_CTLR_ENABLE_G1NS_BIT	BIT_32(1)
#define GICD_CTLR_ENABLE_G1S_BIT	BIT_32(2)
#define GICD_CTLR_ARE_S_BIT			BIT_32(4)
#define GICD_CTLR_ARE_NS_BIT		BIT_32(5)
#define GICD_CTLR_DS_BIT			BIT_32(6)
#define GICD_CTLR_E1NWF_BIT			BIT_32(7)
#define GICD_CTLR_RWP_BIT			BIT_32(31)


#define GICV3_DIST_FRAME_RESERVED(_n, _len)									\
	const uint32_t reserved ## _n[_len]

/**
 * This is the Distributor register frame. This should be created at the base
 * address of the GICv3 distributor, as defined in the device tree.
*/
struct gicv3_dist_frame
{
	uint32_t 	ctlr; 				/* 0x0000 - RW - Distributor Control Register */
	uint32_t 	typer; 				/* 0x0004 - RO - Interrupt Controller Type Register */
	uint32_t 	iidr; 				/* 0x0008 - RO - Distributor Implementor Identification Register */
	uint32_t 	typer2; 			/* 0x000C - RO - Interrupt Controller Type Register 2 */
	uint32_t 	statusr; 			/* 0x0010 - RW - Error Reporting Status Register */

	GICV3_DIST_FRAME_RESERVED(0, 11);	/* 0x0014 - RESERVED / IMPLEMENTATION DEFINED */

	uint32_t 	setspi_nsr; 		/* 0x0040 - WO - Set SPI Register */

	GICV3_DIST_FRAME_RESERVED(1, 1);	/* 0x0044 - RESERVED */

	uint32_t 	clrspi_nsr; 		/* 0x0048 - WO - Clear SPI Register */

	GICV3_DIST_FRAME_RESERVED(2, 1);	/* 0x004C - RESERVED */

	uint32_t 	setspi_sr; 			/* 0x0050 - WO - Set SPI, Secure Register */

	GICV3_DIST_FRAME_RESERVED(3, 1);	/* 0x0054 - RESERVED */

	uint32_t 	clrspi_sr; 			/* 0x0058 - WO - Clear SPI, Secure Register */

	GICV3_DIST_FRAME_RESERVED(4, 9);	/* 0x005C - RESERVED */

	uint32_t 	igroupr[32]; 		/* 0x0080 - RW - Interrupt Group Registers */
	uint32_t 	isenabler[32]; 		/* 0x0100 - RW - Interrupt Set-Enable Registers */
	uint32_t 	icenabler[32]; 		/* 0x0180 - RW - Interrupt Clear-Enable Registers */
	uint32_t 	ispendr[32]; 		/* 0x0200 - RW - Interrupt Set-Pending Registers */
	uint32_t 	icpendr[32]; 		/* 0x0280 - RW - Interrupt Clear-Pending Registers */
	uint32_t 	isactiver[32]; 		/* 0x0300 - RW - Interrupt Set-Active Registers */
	uint32_t 	icactiver[32]; 		/* 0x0380 - RW - Interrupt Clear-Active Registers */

	uint32_t 	ipriorityr[256]; 	/* 0x0400 - RW - Interrupt Priority Registers */
	
	uint32_t 	itargetsr[256]; 	/* 0x0800 - RW - Interrupt Processor Targets Registers */
	uint32_t 	icfgr[64]; 			/* 0x0C00 - RW - Interrupt Configuration Registers */
	uint32_t 	igrpmodr[64]; 		/* 0x0D00 - RW - Interrupt Group Modifier Registers */
	uint32_t 	nsacr[64]; 			/* 0x0E00 - RW - Non-secure Access Control Registers */
	uint32_t 	sgir; 				/* 0x0F00 - WO - Software Generated Interrupt Register */

	GICV3_DIST_FRAME_RESERVED(5, 3);	/* 0x0F04 - RESREVED */

	uint32_t 	cpendsgir[4]; 		/* 0x0F10 - RW - SGI Clear-Pending Registers */
	uint32_t 	spendsgir[4]; 		/* 0x0F20 - RW - SGI Set-Pending Registers */

	GICV3_DIST_FRAME_RESERVED(6, 52);	/* 0x0F30 - RESERVED */

	uint32_t 	igroupre[128];		/* 0x1000 - RW - */
	uint32_t 	isenablere[128];	/* 0x1200 - RW - */
	uint32_t 	icenablere[128];	/* 0x1400 - RW - */
	uint32_t 	ispendre[128];		/* 0x1600 - RW - */
	uint32_t 	icpendre[128];		/* 0x1800 - RW - */
	uint32_t 	isactivere[128];	/* 0x1A00 - RW - */
	uint32_t 	icactivere[128];	/* 0x1C00 - RW - */

	GICV3_DIST_FRAME_RESERVED(7, 128);		/* 0x1E00 - RESERVED */

	uint32_t 	ipriorityre[1024]; 	/* 0x2000 */
	uint32_t 	icfgre[256]; 		/* 0x3000 */
	uint32_t 	igrpmodre[128];		/* 0x3400 */
	uint32_t 	nsacre[64];			/* 0x3600 */

	GICV3_DIST_FRAME_RESERVED(8, 2688);		/* 0x3700 - RESERVED */

	uint32_t	irouter[1024];		/* 0x6100 */

	GICV3_DIST_FRAME_RESERVED(9, 9146);

	uint32_t	pidr2;				/* 0xFFE8 */
};


/*******************************************************************************
 * GICv3 Redistributor Registers and Bit Definitions
 ******************************************************************************/

/**
 * GICR_CTLR, Redistributor Control Register Bits
*/
#define GICR_CTRL_EN_LPIS_BIT	BIT_32(0)
#define GICR_CTRL_CES_BIT		BIT_32(1)
#define GICR_CTRL_IR_BIT		BIT_32(2)
#define GICR_CTRL_RWP_BIT		BIT_32(3)
#define GICR_CTRL_DPG0_BIT		BIT_32(24)
#define GICR_CTRL_DPG1NS_BIT	BIT_32(25)
#define GICR_CTRL_DPG1S_BIT		BIT_32(26)
#define GICR_CTRL_UWP_BIT		BIT_32(31)

/**
 * GICR_WAKER, Redistributor Wake Register Bits
*/
#define GICR_WAKER_PS_BIT		BIT_32(1)
#define GICR_WAKER_CA_BIT		BIT_32(2)


#define GICV3_REDIST_FRAME_RESERVED(_n, _len)								\
	const uint32_t reserved ## _n[_len]


/* Redistributor SGIs frame */
struct gicv3_redist_sgis_frame
{
	GICV3_REDIST_FRAME_RESERVED(0, 32);		/* 0x0000 - RESERVED */

	uint32_t		igroupr[3];		/* 0x0080 - RW - Interrupt Group Registers) */

	GICV3_REDIST_FRAME_RESERVED(1, 29);		/* 0x008C - RESERVED */

	uint32_t 		isenabler[3];	/* 0x0100 - RW - Interrupt Set-Enable Registers */

	GICV3_REDIST_FRAME_RESERVED(2, 29);		/* 0x010C - RESERVED */

	uint32_t 		icenabler[3];	/* 0x0180 - RW - Interrupt Clear-Enable Registers */

	GICV3_REDIST_FRAME_RESERVED(3, 29);		/* 0x018C - RESERVED */

	uint32_t 		ispendr[3];		/* 0x0200 - RW - Interrupt Set-Pending Registers */

	GICV3_REDIST_FRAME_RESERVED(4, 29);		/* 0x020C - RESERVED */

	uint32_t 		icpendr[3];		/* 0x0280 - RW - Interrupt Clear-Pending Registers */

	GICV3_REDIST_FRAME_RESERVED(5, 29);		/* 0x028C - RESERVED */

	uint32_t 		isactiver[3];	/* 0x0300 - RW - Interrupt Set-Active Register */

	GICV3_REDIST_FRAME_RESERVED(6, 29);		/* 0x030C - RESERVED */

	uint32_t 		icactiver[3];	/* 0x0380 - RW - Interrupt Clear-Active Register */

	GICV3_REDIST_FRAME_RESERVED(7, 29);		/* 0x018C - RESERVED */

	uint8_t  		ipriorityr[96]; /* 0x0400 - RW - Interrupt Priority Registers */

	GICV3_REDIST_FRAME_RESERVED(8, 488);	/* 0x0460 - RESERVED */

	uint32_t 		icfgr[6]; 		/* 0x0C00 - RW - Interrupt Configuration Registers */

	GICV3_REDIST_FRAME_RESERVED(9, 58);		/* 0x0C18 - RESERVED */

	uint32_t 		igrpmodr[3]; 	/* 0x0D00 - RW - Interrupt Group Modifier Register */

	GICV3_REDIST_FRAME_RESERVED(10, 61);	/* 0x0D0C - RESERVED */

	uint32_t 		nsacr; 			/* 0x0E00 - RW - Non-secure Access Control Register */
};

/* Redistributor LPIs frame */
struct gicv3_redist_lpis_frame
{
	uint32_t 		ctlr;			/* 0x0000 - RW - Redistributor Control Register */
	uint32_t 		iidr;			/* 0x0004 - RO - Redistributor Implementer Identification Register */
	uint32_t 		typer[2];		/* 0x0008 - RO - Redistributor Type Register */
	uint32_t 		statusr;		/* 0x0010 - RW - Redistributor Status register */
	uint32_t 		waker;			/* 0x0014 - RW - Wake Request Registers */
	uint32_t 		mpamidr;		/* 0x0018 - RO - Reports maximum PARTID and PMG (GICv3.1) */
	uint32_t 		partid;			/* 0x001C - RW - Set PARTID and PMG used for Redistributor memory accesses (GICv3.1) */

	GICV3_REDIST_FRAME_RESERVED(0 ,8);		/* 0x0020 - RESERVED */

	uint64_t 		setlpir;		/* 0x0040 - WO - Set LPI pending */
	uint64_t 		clrlpir;		/* 0x0048 - WO - Set LPI pending */

	GICV3_REDIST_FRAME_RESERVED(1 ,6);		/* 0x0058 - RESERVED */

	uint32_t 		seir; /* 0x0068 - WO - (Note: This was removed from the spec) */

	GICV3_REDIST_FRAME_RESERVED(2, 1);		/* 0x006C - RESERVED */

	uint64_t 		propbaser;		/* 0x0070 - RW - Sets location of the LPI configuration table */
	uint64_t 		pendbaser;		/* 0x0078 - RW - Sets location of the LPI pending table */

	GICV3_REDIST_FRAME_RESERVED(3 ,8);		/* 0x0080 - RESERVED */

	uint64_t 		invlpir; 		/* 0x00A0 - WO - Invalidates cached LPI config */

	GICV3_REDIST_FRAME_RESERVED(4 ,2);		/* 0x00A8 - RESERVED */

	uint64_t 		invallr; 		/* 0x00B0 - WO - Invalidates cached LPI config */

	GICV3_REDIST_FRAME_RESERVED(5 ,2);		/* 0x00B8 - RESERVED */

	uint64_t 		syncr; 			/* 0x00C0 - WO - Redistributor Sync */

	GICV3_REDIST_FRAME_RESERVED(6 ,2);		/* 0x00C8 - RESERVED */
	GICV3_REDIST_FRAME_RESERVED(7 ,1);		/* 0x00D0 - RESERVED */

	uint64_t 		movlpir; 		/* 0x0100 - WO - IMP DEF */

	GICV3_REDIST_FRAME_RESERVED(8 ,2);		/* 0x0108 - RESERVED */

	uint64_t 		movallr; 		/* 0x0110 - WO - IMP DEF */
};

/**
 * This is the Redistributor register frame. This should be created at the base
 * address of the GICv3 redistributor, as defined in the device tree. There is
 * one of these structs for each CPU in the system
*/
struct gicv3_redist_frame
{
	struct gicv3_redist_lpis_frame		lpis	__attribute__((aligned(0x10000)));
	struct gicv3_redist_sgis_frame		sgis	__attribute__((aligned(0x10000)));
};


/*******************************************************************************
 * GICv3 CPU Interface Registers and Bit Definitions
 ******************************************************************************/

/* ICC_SRE_ELn Bit Definitions  */
#define ICC_SRE_SRE_BIT					BIT_32(0)
#define ICC_SRE_DFB_BIT					BIT_32(1)
#define ICC_SRE_DIB_BIT					BIT_32(2)
#define ICC_SRE_EN_BIT					BIT_32(3)		/* EL2/3-only */

/* ICC_IGRPEN1_EL3 Bit Definitions */
#define IGRPEN1_EL3_ENABLE_G1NS_BIT		BIT_32(0)
#define IGRPEN1_EL3_ENABLE_G1S_BIT		BIT_32(1)

/* ICC_CTLR_EL1 Bit Definitions */
#define ICC_CTLR_EOIMODE				BIT_32(1)

/* ICC_IGRPEN0_EL1 Bit Definitions */
#define IGRPEN1_EL1_ENABLE_G0_BIT		BIT_32(0)

// tmp
#define ICC_CTLR_EL1_EOImode_SHIFT	(1)
#define ICC_CTLR_EL1_EOImode_drop_dir	(0U << ICC_CTLR_EL1_EOImode_SHIFT)
#define ICC_CTLR_EL1_EOImode_drop	(1U << ICC_CTLR_EL1_EOImode_SHIFT)
#define ICC_CTLR_EL1_EOImode_MASK	(1 << ICC_CTLR_EL1_EOImode_SHIFT)
#define ICC_CTLR_EL1_CBPR_SHIFT		0
#define ICC_CTLR_EL1_CBPR_MASK		(1 << ICC_CTLR_EL1_CBPR_SHIFT)
#define ICC_CTLR_EL1_PMHE_SHIFT		6
#define ICC_CTLR_EL1_PMHE_MASK		(1 << ICC_CTLR_EL1_PMHE_SHIFT)
#define ICC_CTLR_EL1_PRI_BITS_SHIFT	8
#define ICC_CTLR_EL1_PRI_BITS_MASK	(0x7 << ICC_CTLR_EL1_PRI_BITS_SHIFT)
#define ICC_CTLR_EL1_ID_BITS_SHIFT	11
#define ICC_CTLR_EL1_ID_BITS_MASK	(0x7 << ICC_CTLR_EL1_ID_BITS_SHIFT)
#define ICC_CTLR_EL1_SEIS_SHIFT		14
#define ICC_CTLR_EL1_SEIS_MASK		(0x1 << ICC_CTLR_EL1_SEIS_SHIFT)
#define ICC_CTLR_EL1_A3V_SHIFT		15
#define ICC_CTLR_EL1_A3V_MASK		(0x1 << ICC_CTLR_EL1_A3V_SHIFT)
#define ICC_CTLR_EL1_RSS		(0x1 << 18)
#define ICC_CTLR_EL1_ExtRange		(0x1 << 19)
#define ICC_PMR_EL1_SHIFT		0
#define ICC_PMR_EL1_MASK		(0xff << ICC_PMR_EL1_SHIFT)
#define ICC_BPR0_EL1_SHIFT		0
#define ICC_BPR0_EL1_MASK		(0x7 << ICC_BPR0_EL1_SHIFT)
#define ICC_BPR1_EL1_SHIFT		0
#define ICC_BPR1_EL1_MASK		(0x7 << ICC_BPR1_EL1_SHIFT)
#define ICC_IGRPEN0_EL1_SHIFT		0
#define ICC_IGRPEN0_EL1_MASK		(1 << ICC_IGRPEN0_EL1_SHIFT)
#define ICC_IGRPEN1_EL1_SHIFT		0
#define ICC_IGRPEN1_EL1_MASK		(1 << ICC_IGRPEN1_EL1_SHIFT)
#define ICC_SRE_EL1_DIB			(1U << 2)
#define ICC_SRE_EL1_DFB			(1U << 1)
#define ICC_SRE_EL1_SRE			(1U << 0)

#define ICC_SGI1R_TARGET_LIST_SHIFT	0
#define ICC_SGI1R_TARGET_LIST_MASK	(0xffff << ICC_SGI1R_TARGET_LIST_SHIFT)
#define ICC_SGI1R_AFFINITY_1_SHIFT	16
#define ICC_SGI1R_AFFINITY_1_MASK	(0xff << ICC_SGI1R_AFFINITY_1_SHIFT)
#define ICC_SGI1R_SGI_ID_SHIFT		24
#define ICC_SGI1R_SGI_ID_MASK		(0xfULL << ICC_SGI1R_SGI_ID_SHIFT)
#define ICC_SGI1R_AFFINITY_2_SHIFT	32
#define ICC_SGI1R_AFFINITY_2_MASK	(0xffULL << ICC_SGI1R_AFFINITY_2_SHIFT)
#define ICC_SGI1R_IRQ_ROUTING_MODE_BIT	40
#define ICC_SGI1R_RS_SHIFT		44
#define ICC_SGI1R_RS_MASK		(0xfULL << ICC_SGI1R_RS_SHIFT)
#define ICC_SGI1R_AFFINITY_3_SHIFT	48
#define ICC_SGI1R_AFFINITY_3_MASK	(0xffULL << ICC_SGI1R_AFFINITY_3_SHIFT)


/*******************************************************************************
 * GICv3 Misc Bit Definitions
 ******************************************************************************/

/* Interrupt Group types */
#define GICV3_GROUP_0				1
#define GICV3_GROUP_1_SECURE		2
#define GICV3_GROUP_1_NON_SECURE	3

#endif /* __driver_irq_gicv3_h__ */