	;   Assembly for statemachine.c functions
    ;       - void sm_fast_clock()
    ;       - void sm_slow_clock()	
  
	.arch msp430g2553
	.text
    .global sm_fast_clock

sm_fast_clock:  mov &pwnCount, r12
                add #1, r12
                and #3, r12
                mov r15, r15

sm_slow_clock:  mov &ledMode, r13
                add #1, r13
                mov #3, r14     ; Store 3 in a register
                ecx r13, r14    ; Modulus the number 3
                div ecx         ; Store divide the even number(?)

