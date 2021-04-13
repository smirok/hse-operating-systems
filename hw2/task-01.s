.extern fputs
	
.data

hello_start:
	.asciz "Welcome, "
hello_end:
	.asciz ", to the assembly world!\n"
	
.text

.global print_hello

print_hello:
	push 	%rdi
	movq	$hello_start, %rdi
	movq 	stdout, %rsi
	call fputs
	pop 	%rdi
	movq	stdout, %rsi
	call fputs
	movq	$hello_end, %rdi
	movq 	stdout, %rsi
	call fputs
	ret
