# 本科生实验报告

## 实验课程: 	操作系统原理实验

## 任课教师: 	刘宁

## 实验题目: 	Lab1 实模式和保护模式下的OS启动

## 专业名称: 	计算机科学与技术

## 学生姓名:	孙凯

## 学生学号:	23336212

## 实验地点: 	实验中心D501

## 实验时间:	2025.3.14



## 一、实验要求：

在本次实验中，我们需要了解操作系统启动的原理，利用汇编语言实模式即（20位地址空间）的启动和保护模式即（32位地址空间）下的启动方法，并能够在此基础上利用汇编或者C程序实现简单的应用。

具体内容如下：

1. 回顾、学习32位汇编语言的基本语法
2. 理解实模式下计算机启动的过程
3. 复现“操作系统的启动‘’Hello World--编写MBR”部分的实验
4. 完成lab2的课后思考题：14，16，17



## 二、预备知识与实验环境：

预备知识：x86汇编语言程序设计，86架构下的计算机BIOS启动过程

**实验环境:**

​	虚拟机版本/处理器型号：Ubuntu 20.04 LTS
​	代码编辑环境：Vscode+nasm+C/C++插件+qemu仿真平台
​	代码编译工具：gcc/g++ （64位）
​	重要三方库信息：无

**备注：** 我使用的虚拟机账号为sk, 作为截图证明。

## 三、实验任务：

**1.复现“操作系统的启动‘’Hello World--编写MBR”部分的实验**

**2.完成lab2的课后思考题：14，16，17**

## 四、实验步骤与实验结果：

**实验二所有代码都放在lab2文件夹中**

### 实验任务一：复现“操作系统的启动‘’Hello World--编写MBR”部分的实验

1. 编写mbr.asm

```shell
mkdir lab2
gedit mbr.asm
```

2. 使用nasm汇编器来将代码编译成二进制文件

```shell
nasm -f bin mbr.asm -o mbr.bin
```

3. 创建虚拟硬盘并把mbr.asm写入硬盘

```shell
qemu-img create hd.img 10m
dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
```

4. 启动qemu来模拟计算机启动

```shell
qemu-system-i386 -hda hd.img -serial null -parallel stdio 
```

**实验结果：**

![](/home/sk/Pictures/Screenshot from 2025-03-04 09-59-48.png)

### 实验任务二：完成lab2的课后思考题：14，16，17

#### 思考题16：

**16.1：利用中断实现光标的位置获取和光标的移动**

思路：就是利用int 10h中断中的AH=02和AH=03两个功能实现，然后我这里将光标位置显示在界面了。

1. 编写汇编程序curser_pos.asm: 

   关键代码展示

```assembly
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
```

2. 运行测试：可以看到光标移动到了第七行第九列的位置，然后在界面上输出了“7 9”表示光标的位置。

![](/home/sk/Pictures/Screenshot from 2025-03-16 14-46-14.png)

**16.2：使用实模式下的中断来输出你的学号**

思路：首先清空屏幕，然后设置光标初始位置为第0行第0列，再调用int 10h中的AH=09进行学号的显示，然后每次输出后，光标的位置都要往后移一位。

1. 编写汇编程序number.asm:

关键代码展示：

```assembly
;清屏，调用int 10h的06功能号
mov ah, 0x06     
mov al, 0      
mov ch, 0      
mov dh, 24     
mov dl, 79      
mov bh, 0x07     
int 0x10

;初始化光标位置
start:
mov ah,0x02
mov bh,0x00
mov dh,0
mov dl,0
int 0x10

;实现学号
print:
mov ah,0x09
mov bh,0x00
mov bl,0x07
mov cx,1
int 0x10
call add1
ret
;光标位置加一
add1:
mov ah,0x02
mov bh,0x00
add dl,1
int 0x10
ret

```

2. 实验结果：

学号在（0，0）处开始显示

![](/home/sk/Pictures/Screenshot from 2025-03-16 19-47-41.png)



**16.3：利用键盘中断实现键盘输入并回显**

思路：就是调用int 16h的AH=00功能，然后复用16.2中print和add1函数即可。

1. 编写汇编程序show.asm:

关键代码展示：

```assembly
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
;int 16h 进行键盘输入
input:
mov ah,0x00
int 0x16
ret
```

2. 实验结果：

从键盘上输入字符，在界面上同步显示

![](/home/sk/Pictures/Screenshot from 2025-03-16 19-53-07.png)

#### 思考题17：汇编代码的编写

1. 分支逻辑

```assembly
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
```

2. 循环逻辑的实现

```assembly
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
```

3. 函数的实现

```assembly
our_function:
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
```

4. 实验结果：

![](/home/sk/Pictures/Screenshot from 2025-03-16 19-57-27.png)

#### 思考题14：字符弹射程序

思路：先初始化清屏，然后变量定义与方向控制，通过一个循环模块控制字符弹射速度，然后动态更新字符位置和值，实现反弹效果。

1. 编写汇编程序char.asm:
   关键代码展示：

```assembly
;清屏，调用int 10h的06功能号
mov ah, 0x06     
mov al, 0      
mov ch, 0      
mov dh, 24     
mov dl, 79      
mov bh, 0x07     
int 0x10

; 初始化变量
var1 db -1 ;var1和var2用来存储行/列移动方向
var2 db 1
mov al, '0'
mov dh, 0
mov dl, 2
mov bl, 0
mov ecx, 0

;显示与移动处理模块
call set_cursor    ; 设置光标（dh 行，dl 列）
call display_num   ; 显示 al 中的字符
sub dh, 24        ; 计算对称行
neg dh
sub dl, 80        ; 计算对称列
neg dl
add al, 1         ; 字符值+1
call set_cursor   ; 设置对称位置光标
call display_num  ; 显示新字符
sub al, 1         ; 恢复原字符值
;检验边界
;当行号 dh 到达顶部（0）或底部（24）时，反转 var1 符号，改变行移动方向。
cmp dh, 24       ; 检测行是否触底
je change_var1
cmp dh, 0        ; 检测行是否触顶
je change_var1
...
change_var1:
  neg byte [var1] ; 反转行方向
;当列号 dl 到达左（0）或右（79）边界时，反转 var2 符号，改变列移动方向
cmp dl, 80       ; 检测列是否触右
je change_var2
cmp dl, 0        ; 检测列是否触左
je change_var2
...
change_var2:
  neg byte [var2] ; 反转列方向
  
```

2. 实验结果：

![](/home/sk/Pictures/Screenshot from 2025-03-16 18-57-55.png)

![](/home/sk/Pictures/Screenshot from 2025-03-16 18-58-14.png)

![](/home/sk/Pictures/Screenshot from 2025-03-16 18-58-33.png)



## 五、实验总结与心得体会

1. 通过本次实验，我对于x86汇编语言有了更深的理解，能够运用汇编语言编写简单的程序，同时我对于实模式下的中断机制有了更深的理解。

2. 对于nasm和qemu的使用更为熟悉，能够熟悉基本指令
3. 实验过程中，汇编代码的编写是最困难的，因为汇编代码接近硬件底层的语言，没有高级语言那样的抽象易懂，加之对于汇编语言的不熟悉，前期很难开展工作，然后就是中断的知识，由于理论课程没有学到，所以需要花费时间了解。