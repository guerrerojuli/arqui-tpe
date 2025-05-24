SECTION .data
    message db "Hello, World!", 0xA, 0  ; Message string with newline and null terminator
    message_len equ $ - message - 1      ; Length of the message (excluding null terminator)

SECTION .text
    GLOBAL _sys_write_hello_asm

_sys_write_hello_asm:
    ; Syscall parameters for sys_write (rax=1)
    ; fd (rdi) = 1 (stdout)
    ; buf (rsi) = address of message
    ; count (rdx) = length of message

    mov rax, 1                  ; syscall number for write
    mov rdi, 1                  ; file descriptor (stdout)
    lea rsi, [rel message]      ; pointer to message (RIP-relative)
    mov rdx, message_len        ; message length
    int 0x80                    ; trigger syscall

    ret                         ; Return from function
