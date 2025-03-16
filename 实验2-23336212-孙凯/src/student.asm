%include "head.include"
; you code here
mov eax, [a1] 
mov ebx, [a2] 
your_if:
; put your implementation here
    cmp eax, 12 
    jl f1 
    cmp eax, 24 
    jl f2 
    jmp f3
f1:
    shr eax, 1 
    inc eax 
    mov [if_flag], eax 
    jmp if_end 
f2:
    mov ecx, eax 
    sub ecx, 24 
    neg ecx 
    imul ecx, eax 
    mov [if_flag], ecx 
    jmp if_end 
f3: shl eax, 4 
    mov [if_flag], eax 
    jmp if_end 
if_end: 
your_while:
; put your implementation here
    cmp byte[a2], 12 
    jl end_while 
    call my_random 
    mov ebx, [a2] 
    mov ecx, [while_flag] 
    mov byte[ecx + ebx - 12], al 
    dec byte[a2] 
    jmp your_while 
end_while:

%include "end.include"
your_function:
; put your implementation here
    pushad 
    mov eax, 0 
    loop:
    mov ecx, [your_string] 
    cmp byte[ecx + eax], 0 
    je funcend 
    pushad 
    mov ebx, dword[ecx + eax] 
    push ebx 
    call print_a_char 
    pop ebx 
    popad 
    add eax, 1 
    jmp loop 
funcend:
    popad
    ret 