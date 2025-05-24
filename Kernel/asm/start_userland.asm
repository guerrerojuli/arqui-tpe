section .text

global start_userland
global restart_userland

global USERLAND_CODE_ADDRESS
global USERLAND_DATA_ADDRESS

start_userland:
    mov [userland_start_rsp], rsp

restart_userland:
    push qword 0x0
    push qword [userland_start_rsp]
    push qword 0x202
    push qword 0x8
    push qword [USERLAND_CODE_ADDRESS]
    iretq

section .data
    userland_start_rsp dq 0

section .rodata
    USERLAND_CODE_ADDRESS dq 0xA00000
    USERLAND_DATA_ADDRESS dq 0xB00000
