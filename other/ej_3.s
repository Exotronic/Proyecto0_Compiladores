.text
.global main

main:

	ldr r0, =dir_var_1_superb
	ldr r1, =7
	mov r2, r1
	str r2, [r0]

	ldr r0, =dir_var_2_superb
	ldr r1, =22
	mov r2, r1
	str r2, [r0]

	ldr r1, =5

	ldr r3, =dir_var_1_superb
	ldr r2, [r3]
	add r0, r1, r2
	ldr r1, =t$0
	mov r2, r0 
	str r2, [r1]

	ldr r8, =t$0
	ldr r9, =dir_var_3_superb
	ldr r10, [r8]
	str r10, [r9]

	ldr r1, =5

	ldr r3, =dir_var_2_superb
	ldr r2, [r3]
	add r0, r1, r2
	ldr r1, =t$1
	mov r2, r0 
	str r2, [r1]

	ldr r8, =t$1
	ldr r9, =dir_var_4_superb
	ldr r10, [r8]
	str r10, [r9]

	ldr r2, =dir_var_3_superb
	ldr r1, [r2]

	ldr r3, =dir_var_4_superb
	ldr r2, [r3]
	sub r0, r1, r2
	ldr r1, =t$2
	mov r2, r0 
	str r2, [r1]

	ldr r8, =t$2
	ldr r9, =dir_var_5_superb
	ldr r10, [r8]
	str r10, [r9]

	ldr r2, =dir_var_5_superb
	ldr r1, [r2]

	ldr r2, =12
	sub r0, r1, r2
	ldr r1, =t$3
	mov r2, r0 
	str r2, [r1]

	ldr r8, =t$3
	ldr r9, =dir_var_6_superb
	ldr r10, [r8]
	str r10, [r9]

	ldr r0, =string
	ldr r1, =dir_var_6_superb
	ldr r1, [r1]
	bl printf

	mov r7, #1
	swi 0

.data
dir_var_1_superb: .word 0
dir_var_2_superb: .word 0
t$0: .word 0
dir_var_3_superb: .word 0
t$1: .word 0
dir_var_4_superb: .word 0
t$2: .word 0
dir_var_5_superb: .word 0
t$3: .word 0
dir_var_6_superb: .word 0

string: .asciz "%d\n"
input: .string "%d"
