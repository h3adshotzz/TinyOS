
	.globl	_LowResetVector
_LowResetVector:

	/* disable interrupts */
	msr		DAIFSet, #(0xf)

	/* preserve boot args */
	mov		x27, x0

	/* jump */
	b		_reset_vector


	.align	12
	.globl	_LowExceptionVectorBase
_LowExceptionVectorBase:
	b		.


	.align	2
	.globl	_reset_vector
_reset_vector:
	adr		x0, _LowExceptionVectorBase
	msr		VBAR_EL1, x0

	b		.
