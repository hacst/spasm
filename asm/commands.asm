bits 32

section .spasm_add
spasm_add:
pop ebx
pop eax
add eax, ebx
push eax


section .spasm_mul
spasm_mul:
pop ebx
pop eax
imul eax, ebx
push eax


section .spasm_sub
spasm_sub:
pop ebx
pop eax
sub eax, ebx
push eax


section .spasm_div
spasm_div:
pop ebx
pop eax
xor edx, edx
idiv ebx ; idiv edx:eax, ebx
push eax


section .spasm_les
spasm_les:
pop ebx
pop eax
cmp eax, ebx
jl .less
push dword 0
jmp .done
.less:
push dword 1
.done:


section .spasm_and
spasm_and:
pop ebx
pop eax
and eax, ebx
push eax


section .spasm_equ
spasm_equ:
pop ebx
pop eax
cmp eax, ebx
je .equal
push dword 0
jmp .done
.equal:
push dword 1
.done:


section .spasm_not ; logical !0 == 1 
spasm_not:
pop eax
xor eax, 1
push eax


section .spasm_la
spasm_la:
; Real memory location divided by 4 to enable
; int32 aligned address arithmetic in the
; client application. Requires storage location
; to be 4 byte aligned.

push dword 0xDEADBEAF


section .spasm_lc
spasm_lc:
push dword 0xDEADBEAF


section .spasm_lv
spasm_lv:
pop eax
; Real memory location was divided by 4 to enable
; int32 aligned address arithmetic in the
; client application. Multiply it by 4 to get actual
; location
shl eax, 2
push dword [eax]


section .spasm_str
spasm_str:
pop eax
; Real memory location was divided by 4 to enable
; int32 aligned address arithmetic in the
; client application. Multiply it by 4 to get actual
; location
shl eax, 2
pop ebx
mov [eax], ebx


section .spasm_pri
spasm_pri:
pop eax
call 0xDEADBEAF


section .spasm_rea
spasm_rea:
call 0xDEADBEAF
push eax


section .spasm_jmp
spasm_jmp:
jmp 0xDEADBEAF


section .spasm_jin
spasm_jin:
pop eax
and eax, eax
jz 0xDEADBEAF


section .spasm_nop
spasm_nop:
nop


section .spasm_stp
spasm_stp:
mov eax, 1
xor ebx, ebx
int 80h


