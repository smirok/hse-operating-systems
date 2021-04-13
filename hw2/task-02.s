.global product_of_min_and_max_primes
    
degrees:
    .long 1
    .long 10
    .long 100
    .long 1000
    .long 10000
    .long 100000

product_of_min_and_max_primes:
    movl    $0, %ecx
    movl    $4294967295, %ebx /* MAX_UINT, is not prime */
    jmp     array_loop_statement
    array_loop_break: /* empty */
    mov     %ebx, %eax
    mul     %ecx
    shl     $32, %rdx
    add     %rdx, %rax
    ret

/* array */

array_loop_start:
    cmp     $1, (%rdi)
    je      is_prime
    cmp     $2, (%rdi)
    je      is_prime
    mov     $2, %r8d
    jmp     get_sqrt
    is_prime:
        cmp    %ecx, (%rdi)
        ja     update_max
        cmp    %ebx, (%rdi)
        jb     update_min
    not_prime: /* empty */
    sub     $1, %rsi
    add     $4, %rdi
    jmp     array_loop_statement

update_max:
    mov     (%rdi), %ecx
    jmp     array_loop_start

update_min:
    mov     (%rdi), %ebx
    jmp     array_loop_start

array_loop_statement:
    cmp     $0, %rsi
    jne     array_loop_start
    jmp     array_loop_break

/* prime */

check_prime:
    mov     $0, %edx    
    mov     (%rdi), %eax
    div     %r8d
    cmp     $0, %edx
    je      not_prime
    jmp     prime_loop_statement

prime_loop_statement:
    add     $1, %r8d
    cmpl    (%rdi), %r8d
    je      is_prime
    cmpl    %r9d, %r8d
    jne     check_prime
    jmp     is_prime

get_sqrt: /* Find the closest 10^k larger than (%rdi), result in %r9d */
    mov     degrees, %edx /* degrees[0] = 1 */
    mov     degrees, %eax
    mov     degrees, %r9d
    mul     %edx
    cmp     %eax, (%rdi)
    jb      check_prime
    mov     degrees + 4, %edx /* degrees[1] = 10 */
    mov     degrees + 4, %eax
    mov     degrees + 4, %r9d
    mul     %edx
    cmp     %eax, (%rdi)
    jb      check_prime
    mov     degrees + 8, %edx /* degrees[2] = 100 */
    mov     degrees + 8, %eax
    mov     degrees + 8, %r9d
    mul     %edx
    cmp     %eax, (%rdi)
    jb      check_prime
    mov     degrees + 12, %edx /* degrees[3] = 1000 */
    mov     degrees + 12, %eax
    mov     degrees + 12, %r9d
    mul     %edx
    cmp     %eax, (%rdi)
    jb      check_prime
    mov     degrees + 16, %edx /* degrees[4] = 10000 */
    mov     degrees + 16, %eax
    mov     degrees + 16, %r9d
    mul     %edx
    cmp     %eax, (%rdi)
    jb      check_prime
    mov     degrees + 20, %r9d /* degrees[5] = 100000 */
    jmp     check_prime
