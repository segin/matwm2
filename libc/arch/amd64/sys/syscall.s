.text
.align 16, 0x90
.globl syscall
.type syscall, @function
syscall:
	movq %rdi, %rax
	movq %rsi, %rdi
	movq %rdx, %rsi
	movq %rcx, %rdx
	movq %r8, %r10
	mov %r9, %r8
	mov 8(%rsp), %r9 /* argument 6 is on stack */
	syscall
	cmp $-4095, %rax
	jnc err
	ret
err:
	negq %rax
#ifdef PIC
	movq errno@GOTPCREL(%rip), %rcx
	movl %eax, (%rcx)
#else
	movl %eax, errno(%rip)
#endif /* PIC */
	movq $-1,%rax
	ret
