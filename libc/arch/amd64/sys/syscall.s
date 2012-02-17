
# entrypoint for syscall

.text
	.align 16

.globl syscall
	.type syscall,@function
	
syscall:
	pop %rcx
	pop %rax
	movq %rcx,%r10
	movq %rsp(-16),%rdi
	movq %rsp(-24),%rsi
	movq %rsp(-32),%
	syscall
	push %rcx
	ret		
