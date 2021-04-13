.extern strlen

.global write_to_file

/* r12 - pathname, r13 - text, r14 - text length, r15 - fd */

write_to_file:
	push	%r12 /* PUSH */
	push	%r13
	push 	%r14
	push	%r15

	mov 	%rdi, %r12 /* MOVE PARAMS TO REGISTERS */
	mov 	%rsi, %r13

	mov 	%r13, %rdi /* STRLEN */
	call strlen
	mov 	%rax, %r14

	mov		$85, %rax /* CREAT */
	mov 	$0644, %rsi
	mov 	%r12, %rdi
	syscall

	cmp		$0, %rax /* CHECK ON ERRORS */
	jl		main_err

	mov 	%rax, %r15 /* MOVE FILE DESCRIPTOR TO REGISTER */

	mov 	%r15, %rdi /* WRITE */
	mov 	%r13, %rsi
	mov 	%r14, %rdx
	mov 	$1, %rax
	syscall

	cmp		$0, %rax /* CHECK ON ERRORS */
	jl		err_to_close

	mov 	%r15, %rdi /* CLOSE */
	mov		$3, %rax 
	syscall

	cmp		$0, %rax /* CHECK ON ERRORS */
	jl		main_err

	mov 	$1, %rax

	pop		%r15 /* POP */
	pop		%r14
	pop		%r13
	pop		%r12

	ret

	/* END */

	err_to_close:		
		mov 	%r15, %rdi /* CLOSE */
		mov		$3, %rax 
		syscall

		main_err:
			mov $0, %rax

			pop		%r15 /* POP */
			pop		%r14
			pop		%r13
			pop		%r12

			ret
