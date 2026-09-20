.syntax unified
.arm
.arch armv7-a
.fpu softvfp

.section .text.vector, "ax", %progbits
.global _start
.type _start, %function
_start:
    ldr sp, =stack_top
    bl main
1:  b 1b
.size _start, . - _start

.end