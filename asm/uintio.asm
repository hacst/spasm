bits 32

section .data
prompt:	db "> "
promptlen: equ $-prompt

nanm: db "Invalid input. Please enter an unsigned integer.", 10
nanmlen: equ $-nanm

ofm: db "Number too large. Must be smaller than 2^32.", 10
ofmlen: equ $-ofm

section .bss
strbuf: resb 255
strbuflen: equ $-strbuf

section .text
global _start
_start:

call readunsigned
call writeunsigned

mov ebx, 0 
mov eax, 1
int 80h

; Function for reading an unsigned integer value from stdin.
; Used registers: eax, ebx, ecx, edx, esi, edi
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
mov ecx, strbuf
mov edx, strbuflen
int 80h
; eax contains number of characters in strbuf

cmp eax, 1
je readunsigned

mov esi, strbuf
add eax, esi

xor eax, eax ; Number will be parsed into this register
xor ebx, ebx ; We will only write to the lower byte of ebx
mov edi , 10

parse:
mov bl, byte [esi]

cmp bl, 10 ; Return
je done
cmp bl, '0'
jl errnan
cmp bl, '9'
jg errnan

sub bl, '0'

mul edi ; mul eax, 10
jo overflow
add eax, ebx
jc overflow

inc esi
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

ret

; Function for writing an unsigned integer to stdout.
; Parameter: eax - Value to write to stdout
; Used registers: eax, ebx, ecx, edx, esi
writeunsigned:
mov esi, strbuf + strbuflen ; Use edi as pointer to current string position (writing back to front)
mov ebx, 10

mov [esi], byte 10 ; Newline at end of number
dec esi

generate:
xor edx, edx
div ebx ; div 10

; Store remainder in string
add edx, '0'
mov [esi], dl

dec esi

cmp eax, 0
jnz generate

; Output result
mov eax, 4
mov ebx, 1
mov ecx, esi
mov edx, esi
sub edx, strbuf
int 80h

ret

