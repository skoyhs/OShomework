# 本科生实验报告

## 实验课程: 	操作系统原理实验

## 任课教师: 	刘宁

## 实验题目: 	Lab3 从实模式到保护模式

## 专业名称: 	计算机科学与技术

## 学生姓名:	孙凯

## 学生学号:	23336212

## 实验地点: 	实验中心D501

## 实验时间:	2025.3.26

## 一、实验要求：

在本次实验中，我们要掌握计算机是如何从20位的实模式跳转到32位的保护模式，同时学习如何使用I/O端口和硬件交互。

具体内容如下：

1. 回顾、学习32位汇编语言的基本语法
2. 理解LBA方式是如何读写硬盘
3. 理解计算机的保护模式，掌握如何由实模式跳转到保护模式

## 二、预备知识和实验环境：

预备知识：x86汇编语言程序设计，x86架构下的计算机保护模式的进入，LBA方式读写硬盘。

**实验环境:**

    1. 虚拟机版本/处理器型号：Ubuntu 20.04 LTS
    2. 代码编辑环境：Vscode+nasm+C/C++插件+qemu仿真平台
    3. 代码编译工具：gcc/g++ （64位）
    4.  重要三方库信息：无
    5. 代码程序调试工具：gdb

**备注：** 我使用的虚拟机账号为sk, 作为截图证明。

## 三、实验任务：

- 任务1：课后思考题第9题，复现“加载bootloader”这一节，说说你是怎么做的并提供结果截图，也可以参考Ucore、Xv6等系统源码，实现自己的LBA方式的磁盘访问。
- 任务2：在实验任务一的基础上完成课后思考题第10题。
- 任务3：完成课后思考题第11题，参考https://gitee.com/nelsoncheung/sysu-2023-spring-operating-system/tree/main/appendix/debug_with_gdb_and_qemu
- 任务4：在进入保护模式后，按照如下要求，编写并执行一个自己定义的32位汇编程序，要求简单
  说一说你的实现思路，并提供结果截图。
  使用两种不同的自定义颜色和一个自定义的起始位置(x,y)，使得bootloader加载后，在显示屏
  坐标(x,y)处开始输出自己的学号+姓名拼音首字母缩写，要求相邻字符前景色和背景色必须是相互对
  调的。公告图片中提供了学号为21307233，姓名为宋小宝，自定义位置(12,12)的输出样式，仅供
  参考。代码实现框架可参考实验2和实验3的MBR程序。

## 四、实验步骤和实验结果：

**实验三代码放在lab3文件夹中**

### 任务1：课后思考题第9题，复现“加载bootloader”

- 任务要求：完成课后思考题第9题，并简单总结bootloader的作用是什么。
- 思路分析：

1. 假设BootLoader的大小不大于五个扇区，计算起始和终止的地址。
2. 利用LBA方式读取硬盘mbr后的五个扇区。
3. 创建bootloader.asm，实现输出run bootloader，以此为标志验证进入bootloader成功。

- 实验步骤：

假设bootloader的大小为5个扇区计算bootloader的起始和终止地址。

| name       | start  | length         | end    |
| ---------- | ------ | -------------- | ------ |
| MBR        | 0x7c00 | 0c200(512B)    | 0x7e00 |
| bootloader | 0x7e00 | oca00(512B*5)) | 0x8800 |

1. 编写 `bootloader.asm`和 `mbr.asm`如下所示。
   `bootloader.asm:`

   ```asm
   org 0x7e00
   [bits 16]
   mov ax, 0xb800
   mov gs, ax
   mov ah, 0x03 ;青色
   mov ecx, bootloader_tag_end - bootloader_tag
   xor ebx, ebx
   mov esi, bootloader_tag
   output_bootloader_tag:
       mov al, [esi]
       mov word[gs:bx], ax
       inc esi
       add ebx,2
       loop output_bootloader_tag
   jmp $ ; 死循环
   
   bootloader_tag db 'run bootloader'
   bootloader_tag_end:
   ```

然后我们在 `mbr.asm`处放入使用LBA模式读取硬盘的代码，然后在MBR中加载bootloader到地址0x7e00。

 `mbr.asm:`

```assembly
org 0x7c00
[bits 16]
xor ax, ax ; eax = 0
; 初始化段寄存器, 段地址全部设为0
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; 初始化栈指针
mov sp, 0x7c00
mov ax, 1                ; 逻辑扇区号第0~15位
mov cx, 0                ; 逻辑扇区号第16~31位
mov bx, 0x7e00           ; bootloader的加载地址
load_bootloader:
    call asm_read_hard_disk  ; 读取硬盘
    inc ax
    cmp ax, 5
    jle load_bootloader
jmp 0x0000:0x7e00        ; 跳转到bootloader

jmp $ ; 死循环

asm_read_hard_disk:                           
; 从硬盘读取一个逻辑扇区

; 参数列表
; ax=逻辑扇区号0~15位
; cx=逻辑扇区号16~28位
; ds:bx=读取出的数据放入地址

; 返回值
; bx=bx+512

    mov dx, 0x1f3
    out dx, al    ; LBA地址7~0

    inc dx        ; 0x1f4
    mov al, ah
    out dx, al    ; LBA地址15~8

    mov ax, cx

    inc dx        ; 0x1f5
    out dx, al    ; LBA地址23~16

    inc dx        ; 0x1f6
    mov al, ah
    and al, 0x0f
    or al, 0xe0   ; LBA地址27~24
    out dx, al

    mov dx, 0x1f2
    mov al, 1
    out dx, al   ; 读取1个扇区

    mov dx, 0x1f7    ; 0x1f7
    mov al, 0x20     ;读命令
    out dx,al

    ; 等待处理其他操作
  .waits:
    in al, dx        ; dx = 0x1f7
    and al,0x88
    cmp al,0x08
    jnz .waits                         
    

    ; 读取512字节到地址ds:bx
    mov cx, 256   ; 每次读取一个字，2个字节，因此读取256次即可          
    mov dx, 0x1f0
  .readw:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .readw
      
    ret

times 510 - ($ - $$) db 0
db 0x55, 0xaa
```

2. 编写脚本`start.sh`和`run.sh`

```shell
vim start.sh
chmod +x start.sh #赋权限

vim run.sh
chmod +x run.sh #赋权限
```

`start.sh`

```bash
#!/bin/bash
nasm -f bin mbr.asm -o mbr.bin
nasm -f bin bootloader.asm -o bootloader.bin
qemu-img create hd.img 10m
dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
dd if=bootloader.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc
```

`run.sh`

```bash
#!/bin/bash
qemu-system-i386 -hda hd.img -serial null -parallel stdio
```

3. 运行脚本，进行编译和运行：

```shell
./start.sh
./run.sh
```

#### 实验结果如下：

![](/home/sk/Pictures/Screenshot from 2025-03-26 14-42-32.png)

### 任务2：在实验任务一的基础上完成课后思考题第10题。

- 任务要求：在实验任务1的基础上，完成课后思考题第10题。


- 思路分析：

  计算得到bootloader的CHS参数，bootloader应该从0柱面0磁头2扇区开始访问，共访问5个扇区。

  利用中断直接磁盘服务(Direct Disk Service—— INT 13H) 的功能读取磁盘。

  修改实验任务1中的mbr.asm代码中的函数asm_read_hard_disk为CHS方式加载bootlaoder。

- 实验步骤：

  计算CHS：

  C = LBA // （每柱面磁道数 * 每磁道扇区数）
  H = （LBA // 63）% 每柱面磁道数
  S = （LBA % 每磁道扇区数）+ 1

对应直接磁盘服务(Direct Disk Service——INT 13H) 的参数

1. 改写asm_read_hard_disk：

```assembly
asm_read_hard_disk:                           
   mov ch,0
   mov dh,0
   mov dl,80h
   mov cl,al
   inc cl
   mov ax,0x0205
   int 13h
   add bx,512
   ret
```

2. 运行脚本，进行编译和运行：

```shell
./start.sh
./run.sh
```

#### 实验结果如下：

![](/home/sk/Pictures/Screenshot from 2025-03-26 15-38-20.png)

### 任务3：

- 实验要求：复现“进入保护模式”一节，使用gdb或其他debug工具在进入保护模式的4个重要步骤上设置断点，并结合代码、寄存器的内容等来分析这4个步骤，最后附上结果截图。
- 思路分析：
  1. 准备GDT，用lgdt指令加载GDTR信息。
  2. 打开第21根地址线。
  3. 开启cr0的保护模式标志位。
  4. 远跳转，进入保护模式。
- 实验步骤：

1. 编写`bootloader.asm`和`mbr.asm`
2. 编写`makefile`:

```makefile
run:
	@qemu-system-i386 -hda hd1.img -serial null -parallel stdio 
debug:
	@qemu-system-i386 -s -S -hda hd1.img -serial null -parallel stdio &
	@sleep 1
	@gnome-terminal -e "gdb -q -x gdbinit"
build:
	@nasm -g -f elf32 mbr.asm -o mbr.o
	@ld -o mbr.symbol -melf_i386 -N mbr.o -Ttext 0x7c00
	@ld -o mbr.bin -melf_i386 -N mbr.o -Ttext 0x7c00 --oformat binary

	@nasm -g -f elf32 bootloader.asm -o bootloader.o
	@ld -o bootloader.symbol -melf_i386 -N bootloader.o -Ttext 0x7e00
	@ld -o bootloader.bin -melf_i386 -N bootloader.o -Ttext 0x7e00 --oformat binary

	@dd if=mbr.bin of=hd1.img bs=512 count=1 seek=0 conv=notrunc
	@dd if=bootloader.bin of=hd1.img bs=512 count=5 seek=1 conv=notrunc
clean:
	@rm -fr *.bin *.o *.symbol
```

3. 直接`make build`和`make run`编译运行，生成符号表。

```shell	
make build
make run
```

![](/home/sk/Pictures/Screenshot from 2025-03-26 15-52-05.png)

4. 用`make debug`命令打开gdb调试界面

```shell
make debug
```

![](/home/sk/Pictures/Screenshot from 2025-03-26 17-34-22.png)

5. 在`bootloader.asm` line 40打上断点，然后查看pgdt寄存器中的值，为39，说明程序成功初始化描述符表寄存器GDTR。再查看五个段描述符的值，符合预期。

![](/home/sk/Pictures/Screenshot from 2025-03-26 17-45-44.png)

![](/home/sk/Pictures/Screenshot from 2025-03-26 17-46-37.png)

![](/home/sk/Pictures/Screenshot from 2025-03-26 18-50-15.png)

6. 在`bootloader.asm` line 44处打上断点，查看第二十一根地址线是否打开，可以观察到0x92的值和al寄存器的值与程序预期执行结果一致，说明第21根地址线已经打开。

![](/home/sk/Pictures/Screenshot from 2025-03-26 17-48-07.png)

![](/home/sk/Pictures/Screenshot from 2025-03-26 17-51-04.png)

7. 在`bootloader.asm` line 45处打上断点，然后单步执行ni，观察eax寄存器和cr0的值的变化,代码执行前cr0 是 16，执行后是 17，说明已经开启cr0的保护模式标志位。

![](/home/sk/Pictures/Screenshot from 2025-03-26 17-58-33.png)

8. 在`bootloader.asm` line 50和line 76处打上断点，然后观察各个寄存器的值，发现eip正确执行到对应的指令，说明远跳转成功，成功进入保护模式。

![](/home/sk/Pictures/Screenshot from 2025-03-26 18-03-58.png)

![](/home/sk/Pictures/Screenshot from 2025-03-26 18-05-07.png)

### 任务4：

- 任务要求：在进入保护模式后，按照如下要求，编写并执行一个自己定义的32位汇编程序，要求简单
  说一说你的实现思路，并提供结果截图。
  使用两种不同的自定义颜色和一个自定义的起始位置(x,y)，使得bootloader加载后，在显示屏
  坐标(x,y)处开始输出自己的学号+姓名拼音首字母缩写，要求相邻字符前景色和背景色必须是相互对
  调的。公告图片中提供了学号为21307233，姓名为宋小宝，自定义位置(12,12)的输出样式，仅供
  参考。代码实现框架可参考实验2和实验3的MBR程序。
- 思路分析：

1. 自定义显示位置
2. 将前景色和背景色进行替换
3. 无限循环程序

- 实验步骤：

1. 编写学号姓名显示模块：

```assembly
;显示学号姓名
mov ecx,id_student_end - id_student
mov ebx, (80*5+35)*2
mov esi,id_student
mov ah,0x71
output_id_student:
    mov al,[esi]
    mov word[gs:ebx],ax
;交换前景色和背景色
    mov dl, ah               
    and dl, 0x0F             
    mov dh, ah               
    shr dh, 4          
   
    shl dl, 4                
    or dl, dh                
    mov ah, dl             
    add ebx, 2
    inc esi
    loop output_id_student
  
jmp $ ; 死循环

id_student db '23336212sk'
id_student_end:
```

2. 使用`make build`和`make run`编译执行程序：

```shell
make build
make run
```

![](/home/sk/Pictures/Screenshot from 2025-03-26 19-59-37.png)

这里我设置初始颜色为白底蓝字，然后进行颜色反转为蓝底白字，在(5,35)处输出我的学号与姓名`23336212sk`。

## 五、实验总结和心得体会

1. 学习了如何从实模式跳转到保护模式，并在平坦模式下运行32位程序。除此之外，也学习到如何使用I/O端口与硬件交互。
2. 学习了LBA和CHS两种硬盘读取方式，同时掌握了如何使用GDB进行源码级的调试。

## 六、参考材料：

1. https://blog.csdn.net/jackailson/article/details/84109450
2. https://blog.csdn.net/jinking01/article/details/105192830
