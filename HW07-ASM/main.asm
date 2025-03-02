    bits 64
    extern malloc, puts, printf, fflush, abort
    global main

    section   .data
empty_str: db 0x0
int_format: db "%ld ", 0x0
data: dq 4, 8, 15, 16, 23, 42
data_length: equ ($-data) / 8

    section   .text
;;; print_int proc
print_int:
    push rsi
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rsi, rdi
    mov rdi, int_format
    xor rax, rax
    call printf

    xor rdi, rdi
    call fflush

    mov rsp, rbp
    pop rbp
    pop rsi
    ret

;;; p proc
p:
    mov rax, rdi
    and rax, 1
    ret

;;; add_element proc
;;; rdi - значение
;;; rsi - указатель на предыдущее
add_element:
    push rbp
    push rbx
    push r14
    mov rbp, rsp
    sub rsp, 16

    mov r14, rdi
    mov rbx, rsi

    mov rdi, 16
    call malloc
    test rax, rax
    jz abort

    mov [rax], r14
    mov [rax + 8], rbx

    mov rsp, rbp
    pop r14
    pop rbx
    pop rbp
    ret

;;; m proc - похоже, что мы идем в обратном порядке по нодам и печатаем
;;; в rdi, rbx - rax - адрес последнего добавленного/предыдущего Node
;;; в rsi - адрес print_int
m:
;;; если rdi - адрес последнего добавленного/предыдущего Node - нулевой, то прыгаем в outm
    test rdi, rdi
    jz outm
;;; если нет, то
    push rbx
    mov rbx, rdi

;;; значение по адресу в rdi перемещаем в rdi, вызываем print_int
    mov rdi, [rdi]
    call rsi

;;; в rdi перемещаем адрес предыдущего Node
    mov rdi, [rbx + 8]
    call m

    pop rbx

outm:
    ret

;;; f proc
;;; в rdi - последний Node из списка
;;; в rdx - p
;;; в rsi - 0
f:
    test rdi, rdi
    jz outf

    push rbx
    push r13

    mov rbx, rdi
    mov r13, rdx

    mov rdi, [rdi]
    call rdx
    test rax, rax
    jz ff

    call add_element
    mov rsi, rax

ff:
    mov rdi, [rbx + 8]
    mov rdx, r13
    call f

    pop r13
    pop rbx

outf:
    mov rax, rsi
    ret

;;; main proc
main:
    push rbx

    xor rax, rax
    mov rbx, data_length

adding_loop:
    mov rdi, [data - 8 + rbx * 8]
    mov rsi, rax
    call add_element
    dec rbx
    jnz adding_loop

;;; в rax - адрес последнего добавленного Node
    mov rbx, rax

    mov rdi, rax
    mov rsi, print_int
    call m
;;; перенос строки
    mov rdi, empty_str
    call puts

    mov rdx, p
;;; обнуление rsi
    xor rsi, rsi
;;; в rbx - последний Node из списка
    mov rdi, rbx
    call f

    mov rdi, rax
    mov rsi, print_int
    call m

    mov rdi, empty_str
    call puts

    pop rbx

    xor rax, rax
    ret
