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
 *	Name:	timer.S
 *	Desc:	ARM Generic Timer.
 */

	.align		2
	.section	".text"
	.globl		arm64_timer_init
arm64_timer_init:
	msr		CNTP_TVAL_EL0, x0		// CNTP_TVAL_EL0 = initial_timer_value
	mov		x0, #1
	msr		CNTP_CTL_EL0, x0		// Enable the timer
	ret

	.globl		arm64_timer_reset
arm64_timer_reset:
	msr		CNTP_TVAL_EL0, x0		// CNTP_TVAL_EL0 = reset_timer_value
	ret