.text
.align 16, 0x90
.globl _start
.type _start, @function
_start:
	xorl %ebp, %ebp
	popq %rdi
	movq %rsp, %rsi
	andq $0xFFFFFFFFFFFF0000, %rsp
	callq main
	movq %rax, %rdi
	mov $60, %rax
	syscall
	ret
