%include "uintio.asm"

section .text
global _start
_start:

call readunsigned
call writeunsigned

mov ebx, 0
mov eax, 1
int 80h

