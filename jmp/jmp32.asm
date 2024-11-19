    global _start
    section .text
_start:
    mov eax, 0x01ffffff ; i386: b8 ff ffff 01
    jmp eax ; i386: ff e0
