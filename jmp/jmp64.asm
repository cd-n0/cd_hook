    global _start
    section .text
_start:
    mov rax, 0x01ffffffffffffff ; x86_64: 48 b8 ffff ffff ffff ff01
    jmp rax ; x86_64: ff e0
