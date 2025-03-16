org 0x7c00
[bits 16]
xor ax,ax
;初始化段寄存器，段地址全部设为0
mov ds,ax
mov ss,ax
mov es,ax
mov fs,ax
mov gs,ax

;初始化栈指针
mov sp,0x7c00
mov ax,0xb800
mov gs,ax

call start
jmp $

start:
xor dx,dx
;光标移动
mov ah,0x02
mov bh,0x00
mov dh,7
mov dl,9
int 0x10

;获取光标位置
mov ah,0x03 ;设置功能号为读取坐标位置
mov bh,0x00 ;设置页面号
int 0x10    ;产生中断
;显示光标位置

add dh,48
add dl,48

mov ah,0x07
mov al,dh
mov [gs:2*(12*80+40)],ax

mov al,' '
mov [gs:2*(12*80+41)],ax

mov al,dl
mov [gs:2*(12*80+42)],ax

ret

times 510 - ($ - $$) db 0
db 0x55, 0xaa