.syntax unified
.cpu cortex-m0
.fpu softvfp

.thumb

//.global PendSV_Handler
//.type PendSV_Handler, %function
//PendSV_Handler:
.global SVC_Handler
.type SVC_Handler, %function
SVC_Handler:
	/* Disable interrupts: */
	cpsid	i

    /*
    If global variable pendsv_restore > 0 jump to restore
    */
    ldr r0, =pendsv_restore
    ldr r0, [r0]
    cmp r0, #0
    bgt b_restore

	/*
	Exception frame saved by the NVIC hardware onto stack:
	+------+
	|      | <- SP before interrupt (orig. SP)
	| xPSR |
	|  PC  |
	|  LR  |
	|  R12 |
	|  R3  |
	|  R2  |
	|  R1  |
	|  R0  | <- SP after entering interrupt (orig. SP + 32 bytes)
	+------+
	Registers saved by the software (PendSV_Handler):
	+------+
	|  R7  |
	|  R6  |
	|  R5  |
	|  R4  |
	|  R11 |
	|  R10 |
	|  R9  |
	|  R8  | <- Saved SP (orig. SP + 64 bytes)
	+------+
	*/

	/* Save registers R4-R11 (32 bytes) onto current PSP (process stack
	   pointer) and make the PSP point to the last stacked register (R8):
	   - The MRS/MSR instruction is for loading/saving a special registers.
	   - The STMIA inscruction can only save low registers (R0-R7), it is
	     therefore necesary to copy registers R8-R11 into R4-R7 and call
	     STMIA twice. */
	//mrs	r0, msp

    /*  The global storage for the registers is `volatile uint32_t registers[16]`
        in checkpoint.c
        There are some pre-computed pointers for use here:
            - registers_top
            - registers_top_sw_save
    */

    /*
    Checkpoint
    */
    ldr r0, =registers_top
    ldr r0, [r0]
	subs	r0, #16
	stmia	r0!,{r4-r7}
	mov	r4, r8
	mov	r5, r9
	mov	r6, r10
	mov	r7, r11
	subs	r0, #32
	stmia	r0!,{r4-r7}
	subs	r0, #16

    /* Copy registers saved to msp to the registers array */
	mrs	r1, msp

	ldmia	r1!,{r4-r7} // r0, r1, r2, r3
    subs    r0, #16
    stmia   r0!,{r4-r7} // push r0, r1, r2, r3 to the register array

	ldmia	r1!,{r4-r7} // r12, LR, PC, xPSR
    subs    r0, #32
    stmia   r0!,{r4-r7} // push r12, LR, PC, xPSR to the register array

    subs    r0, #16

    mrs     r1, msp
    subs    r0, #4
    str     r1, [r0]

    b       b_cleanup

    /*
    Restore
    */
    b_restore:

    // Restore the SP from the checkpoint
    ldr r0, =registers // sp is registers[0]
    ldr r1, [r0]

    // r0 = registers
    // r1 = msp

    adds    r0, #4 // point to registers[1] (end of checkpoint)

	ldmia	r0!,{r4-r7} // load r12, LR, PC, xPSR
    adds    r1, #16 // add 32 bytes to end at the SP before the ISR, substr 16 bytes to end where we need to push r12-xPSR
    stmia   r1!,{r4-r7} // push r12, LR, PC, xPSR to the stack before the ISR

	ldmia	r0!,{r4-r7} // load r0, r1, r2, r3
    subs    r1, #32
    stmia   r1!,{r4-r7} // push r0, r1, r2, r3 to the stack before the ISR

    subs    r1, #16
    msr     msp, r1

    adds    r0, #16
	ldmia	r0!,{r4-r7} // load r8, r9, r10, r11
	mov	    r8, r4
	mov	    r9, r5
	mov	    r10, r6
	mov	    r11, r7

    subs    r0, #32
	ldmia	r0!,{r4-r7} // load r4, r5, r6, r7

    b_cleanup:

	/* EXC_RETURN - Thread mode with PSP: */
	ldr r0, =0xFFFFFFF1
	/* Enable interrupts: */
	cpsie	i
	bx	r0

//.size PendSV_Handler, .-PendSV_Handler
.size SVC_Handler, .-SVC_Handler



//PendSV_Handler:
//	/* Disable interrupts: */
//	cpsid	i
//
//	/*
//	Exception frame saved by the NVIC hardware onto stack:
//	+------+
//	|      | <- SP before interrupt (orig. SP)
//	| xPSR |
//	|  PC  |
//	|  LR  |
//	|  R12 |
//	|  R3  |
//	|  R2  |
//	|  R1  |
//	|  R0  | <- SP after entering interrupt (orig. SP + 32 bytes)
//	+------+
//	Registers saved by the software (PendSV_Handler):
//	+------+
//	|  R7  |
//	|  R6  |
//	|  R5  |
//	|  R4  |
//	|  R11 |
//	|  R10 |
//	|  R9  |
//	|  R8  | <- Saved SP (orig. SP + 64 bytes)
//	+------+
//	*/
//
//	/* Save registers R4-R11 (32 bytes) onto current PSP (process stack
//	   pointer) and make the PSP point to the last stacked register (R8):
//	   - The MRS/MSR instruction is for loading/saving a special registers.
//	   - The STMIA inscruction can only save low registers (R0-R7), it is
//	     therefore necesary to copy registers R8-R11 into R4-R7 and call
//	     STMIA twice. */
//	mrs	r0, psp
//	subs	r0, #16
//	stmia	r0!,{r4-r7}
//	mov	r4, r8
//	mov	r5, r9
//	mov	r6, r10
//	mov	r7, r11
//	subs	r0, #32
//	stmia	r0!,{r4-r7}
//	subs	r0, #16
//
//	/* Save current task's SP: */
//	//ldr	r2, =os_curr_task
//	ldr	r1, [r2]
//	str	r0, [r1]
//
//	/* Load next task's SP: */
//	//ldr	r2, =os_next_task
//	ldr	r1, [r2]
//	ldr	r0, [r1]
//
//	/* Load registers R4-R11 (32 bytes) from the new PSP and make the PSP
//	   point to the end of the exception stack frame. The NVIC hardware
//	   will restore remaining registers after returning from exception): */
//	ldmia	r0!,{r4-r7}
//	mov	r8, r4
//	mov	r9, r5
//	mov	r10, r6
//	mov	r11, r7
//	ldmia	r0!,{r4-r7}
//	msr	psp, r0
//
//	/* EXC_RETURN - Thread mode with PSP: */
//	ldr r0, =0xFFFFFFFD
//
//	/* Enable interrupts: */
//	cpsie	i
//
//	bx	r0
//
//.size PendSV_Handler, .-PendSV_Handler
