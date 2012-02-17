
# entrypoint for syscall

.text
	.align 4

.globl syscall
	.type syscall,@function
	
syscall:
	pop %ecx
	pop %eax
	push %ecx
	int $0x80
	push %ecx
	ret		
