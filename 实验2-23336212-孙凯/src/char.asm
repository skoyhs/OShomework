org 0x7c00
[bits 16]
xor ax, ax   
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

mov sp, 0x7c00
mov ax, 0xb800
mov gs, ax

; 清屏：使用更简洁的方式
mov ah, 0x06
mov al, 0
mov ch, 0
mov dh, 24
mov dl, 79
mov bh, 0x07
int 0x10

; 初始化变量
var1 db -1
var2 db 1
mov al, '0'
mov dh, 0
mov dl, 2
mov bl, 0
mov ecx, 0

loop:
    cmp ecx, 100000000
    je function
    add ecx, 1
    jmp loop

function:
    call set_cursor
    call display_num
    sub dh, 24
    neg dh
    sub dl, 80
    neg dl
    add al, 1
    call set_cursor
    call display_num
    sub al, 1
    neg dl
    add dl, 80
    neg dh
    add dh, 24

    cmp dh, 24
    je change_var1
    cmp dh, 0
    je change_var1
    jmp continue1

change_var1:
    neg byte[var1]
    jmp continue1

continue1:
    add dh, [var1]
    cmp dl, 80
    je change_var2
    cmp dl, 0
    je change_var2
    jmp continue2

change_var2:
    neg byte[var2]
    jmp continue2

continue2:
    add dl, [var2]
    add bl, 1
    mov ecx, 0
    cmp al, '8'
    je update_al
    add al, 2
    jmp loop

update_al:
    mov al, '0'
    jmp loop

set_cursor:
    mov ah, 02h
    mov bh, 0x0
    int 10h
    ret

display_num:
    mov ah, 09h
    mov bh, 0
    mov cx, 1
    int 10h
    ret
