.text
.global main

main:

	ldr r0, =dir_a
	ldr r1, =1
	mov r2, r1
	str r2, [r0]

	ldr r0, =dir_b
	ldr r1, =0
	mov r2, r1
	str r2, [r0]

	ldr r0, =input
	ldr r1, =dir_b
	bl scanf

	ldr r2, =dir_b
	ldr r1, [r2]

	ldr r3, =dir_a
	ldr r2, [r3]
	add r0, r1, r2
	ldr r1, =t$0
	mov r2, r0 
	str r2, [r1]

	ldr r8, =t$0
	ldr r9, =dir_c
	ldr r10, [r8]
	str r10, [r9]

	ldr r2, =dir_c
	ldr r1, [r2]

	ldr r3, =dir_a
	ldr r2, [r3]
	add r0, r1, r2
	ldr r1, =t$1
	mov r2, r0 
	str r2, [r1]

	ldr r8, =t$1
	ldr r9, =dir_d
	ldr r10, [r8]
	str r10, [r9]

	ldr r0, =string
	ldr r1, =dir_c
	ldr r1, [r1]
	bl printf

	ldr r0, =string
	ldr r1, =dir_d
	ldr r1, [r1]
	bl printf

	mov r7, #1
	swi 0

.data
dir_a: .word 0
dir_b: .word 0
t$0: .word 0
dir_c: .word 0
t$1: .word 0
dir_d: .word 0

string: .asciz "%d\n"
input: .string "%d"
