GLOBAL sys_read
GLOBAL sys_write

%macro syscall 1
    push rbp
    mov rbp, rsp

    mov rax, %1 
    mov r10, rcx
    int 0x80

    mov rsp, rbp
    pop rbp
    ret
%endmacro

sys_read:
    syscall 0

sys_write:
    syscall 1