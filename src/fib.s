	.syntax unified
	.arch armv7-m
	.text
	.align 2
	.thumb
	.thumb_func

	.global fibonacci
	.type fibonacci, function

fibonacci:
	@ ADD/MODIFY CODE BELOW
	@ PROLOG
	push {r4, r5, lr} // remove r3 because it is used for holding argument values to the subroutine

	subs r4,r0,#0
	cmp r0,#0
	it le
	ble .L3

	cmp r4 ,#1
	it eq
	beq .L4

	sub r0 , r4, #1
	bl fibonacci

	mov r5,r0
	sub r0,r4,#2
	bl fibonacci

	adds r0,r5,r0

	pop {r4, r5, pc}		@EPILOG

	@ END CODE MODIFICATION
.L3:
	mov r0, #0			@ R0 = 0
	pop {r4, r5, pc}		@ EPILOG

.L4:
	mov r0, #1			@ R0 = 1
	pop {r4, r5, pc}		@ EPILOG

	.size fibonacci, .-fibonacci
	.end
