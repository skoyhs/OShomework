org 0x7c00
[bits 16]
xor ax,ax
mov ds,ax
mov ss,ax
mov es,ax
mov fs,ax
mov gs,ax

mov sp,0x7c00

mov ah, 0x06     ;
mov al, 0      
mov ch, 0      
mov dh, 24     
mov dl, 79      
mov bh, 0x07     
int 0x10

call start
jmp $

start:
mov ah,0x02
mov bh,0x00
mov dh,0
mov dl,0
int 0x10

mov al,'2'
call print

mov al,'3'
call print

mov al,'3'
call print

mov al,'3'
call print

mov al,'6'
call print

mov al,'2'
call print

mov al,'1'
call print

mov al,'2'
call print
ret
print:

mov ah,0x09
mov bh,0x00
mov bl,0x07
mov cx,1
int 0x10
call add1
ret

add1:
mov ah,0x02
mov bh,0x00
add dl,1
int 0x10
ret

 times 510 - ($ - $$) db 0
 db 0x55, 0xaa