bits 32

section .data
prompt:	db "> "
promptlen: equ $-prompt

nanm: db "Invalid input. Please enter an unsigned integer.", 10
nanmlen: equ $-nanm

ofm: db "Number too large. Must be smaller than 2^32.", 10
ofmlen: equ $-ofm

section .bss
inputbuf: resb 255
inputbuflen: equ $-inputbuf

section .text
global _start
_start:

call readunsigned

; Return it for testing reasons
mov ebx, eax
mov eax, 1
int 80h

; Function for reading an unsigned integer value from the prompt
; Used registers: eax, ebx, ecx, edx, esi
; Return value: eax

readunsigned:

; Output prompt
mov eax, 4 ; sys_write
mov ebx, 1 ; stdout
mov ecx, prompt
mov edx, promptlen
int 80h

; Read input from stdin
mov eax, 3 ; sys_read
mov ebx, 0 ; stdin
mov ecx, inputbuf
mov edx, inputbuflen
int 80h
; eax contains number of characters in inputbuf

cmp eax, 1
je readunsigned

mov esi, inputbuf
add eax, esi

xor ecx, ecx ; Number will be parsed into this register
xor ebx, ebx ; We will only write to the lower byte of ebx
parse:
mov bl, byte [esi]

cmp bl, 10 ; Return
je done
cmp bl, '0'
jl errnan
cmp bl, '9'
jg errnan

sub bl, '0'

imul ecx, 10
jo overflow
add ecx, ebx
jo overflow

inc esi
cmp esi, eax
je overflow ; Shouldn't hit this before return

jmp parse

overflow:
mov eax, 4
mov ebx, 1
mov ecx, ofm
mov edx, ofmlen
int 80h

jmp readunsigned

errnan:
mov eax, 4
mov ebx, 1
mov ecx, nanm
mov edx, nanmlen
int 80h

jmp readunsigned


done:
; Copy result to output register
mov eax, ecx

ret


