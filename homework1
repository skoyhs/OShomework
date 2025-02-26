# 本科生实验报告

## 实验课程: 	操作系统原理实验

## 任课教师: 	刘宁
## 实验题目: 	Lab1 编译内核利用已有内核构建OS

## 专业名称: 	计算机科学与技术

## 学生姓名:	孙凯

## 学生学号:	23336212

## 实验地点: 	实验中心D501

## 实验时间:	2025.2.26



## 一、实验要求：

​	在本次实验中，同学们会熟悉现有 Linux 内核的编译过程和启动过程， 并在自行编译内核的基础上构建简单应用并启动。同时，同
学们会利用精简的 Busybox 工具集构建简单的 OS， 熟悉现代操作系统的构建过程。 此外，同学们会熟悉编译环境、相关工具集，并
能够实现内核远程调试。具体内容如下：

1. 环境配置
2. 编译Linux内核
3. 启动内核并调试
4. 制作Initramfs
5. 加载Initramfs并再次使用gdb调试
6. 编译并启动Busybox

## 二、预备知识与实验环境：

​	预备知识：x86汇编语言程序设计、Linux系统命令行工具

​	**实验环境:**

​	虚拟机版本/处理器型号：Ubuntu 20.04 LTS
​	代码编辑环境：Vscode+nasm+C/C++插件
​	代码编译工具：gcc/g++ （64位）
​	重要三方库信息：无

## 三、实验任务：

	1. 任务一：完成环境配置
	1. 任务二：完成Linux内核编译
	1. 任务三：完成内核启动并使用GDB调试
	1. 任务四：完成Initramfs的制作，并加载内核再次使用GDB调试
	1. 任务五：完成Busybox编译和启动

## 四、实验步骤与实验结果：

	### 实验任务一：

1. 下载 Vmware，安装 Ubuntu 虚拟机。

​	![](/home/sk/Pictures/Screenshot from 2025-02-26 20-05-48.png)

2. 换源：

​	输入以下指令即可

```shell
sudo mv /etc/apt/sources.list /etc/apt/sources.list.backup
sudo gedit /etc/apt/sources.list
sudo apt update  ##检查是否换源成功
```

3. 配置C/C++环境

```shell
sudo apt install binutils
sudo apt install gcc
```

​    查看`gcc`是否安装。

```shell
gcc -v
```

   若输出gcc的版本号则表明安装成功。

4. 安装其他工具

```shell
sudo apt install qemu
sudo apt install cmake
sudo apt install libncurses5-dev
sudo apt install bison
sudo apt install flex
sudo apt install libssl-dev
sudo apt install libc6-dev-i386
sudo apt install gcc-multilib 
sudo apt install g++-multilib
```

安装nasm汇编工具：

```shell
wget https://github.com/netwide-assembler/nasm/releases/download/v2.15.01/nasm-2.15.01.tar.gz
##解压
tar -xvzf nasm-2.15.01.tar.gz
cd nasm-2.15.01
##编译安装nasm
./configure
make
sudo make install
```

### 实验任务二：

任务要求：完成Linux内核编译

1. 下载Linux内核: 我这里下载的是Linux-5.10.234版本

![](/home/sk/Pictures/Screenshot from 2025-02-26 20-21-34.png)

2. 编译Linux:

```shell
make i386_defconfig
make menuconfig
'编译'
make -j8
```

编译结果：

![](/home/sk/Pictures/Screenshot from 2025-02-24 17-00-51.png)

### 实验任务三：

任务要求：完成内核启动并使用GDB调试

1. 使用`qemu`启动内核并开启远程调试。

   ```shell
   qemu-system-i386 -kernel linux-5.10.19/arch/x86/boot/bzImage -s -S -append "console=ttyS0" -nographic
   ```

2. 在另外一个Terminal下启动gdb。

```shell
gdb
###在gdb下，加载符号表
file linux-5.10.234/vmlinux
###在gdb下，连接已经启动的qemu进行调试。
target remote:1234
###在gdb下，为start_kernel函数设置断点。
break start_kernel
###在gdb下，输入`c`运行。
c
```

![](/home/sk/Pictures/Screenshot from 2025-02-26 15-22-46.png)

### 实验任务四：

任务要求：完成Initramfs的制作，并加载内核再次使用GDB调试

 	1. 编写helloworld.c文件

```shell
vim helloworld.c
```

```c
#include <stdio.h>

void main()
{
    printf("lab1: Hello World\n");
    fflush(stdout);
    /* 让程序打印完后继续维持在用户态 */
    while(1);
}
```

2. 编译helloworld.c文件

```shell
gcc -o helloworld -m32 -static helloworld.c
```

3. 用cpio打包initramfs。

   ```shell
   echo helloworld | cpio -o --format=newc > hwinitramfs
   ```

   启动内核，并加载initramfs。

   ```shell
   qemu-system-i386 -kernel linux-5.10.19/arch/x86/boot/bzImage -initrd hwinitramfs -s -S -append "console=ttyS0 rdinit=helloworld" -nographic
   ```

   重复上面的gdb的调试过程，可以看到gdb中输出了`lab1: Hello World\n`

任务结果：

![](/home/sk/Pictures/Screenshot from 2025-02-26 15-31-04.png)

### 实验任务五：

任务要求：完成Busybox编译和启动

1. 编译Busybox

```shell
make defconfig
make menuconfig
make -j8
make install
```

![](/home/sk/Pictures/Screenshot from 2025-02-26 20-46-30.png)

2.  制作Initramfs

将安装在_install目录下的文件和目录取出放在`~/lab1/mybusybox`处。

```shell
cd ~/lab1
mkdir mybusybox
mkdir -pv mybusybox/{bin,sbin,etc,proc,sys,usr/{bin,sbin}}
cp -av busybox-1_33_0/_install/* mybusybox/
cd mybusybox
```

编写shell脚本作为init

```shell
gedit init
```

输入内容如下：

![](/home/sk/Pictures/Screenshot from 2025-02-26 20-50-47.png)

将x86-busybox下面的内容打包归档成cpio文件，以供Linux内核做initramfs启动执行。

```shell
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ~/Lab1/initramfs-busybox-x86.cpio.gz
```

3. 加载busybox

```shell
cd ~/Lab1
qemu-system-i386 -kernel linux-5.10.234/arch/x86/boot/bzImage -initrd initramfs-busybox-x86.cpio.gz -nographic -append "console=ttyS0"
```

任务结果：

![](/home/sk/Pictures/Screenshot from 2025-02-26 15-56-46.png)



## 五、实验总结与心得体会

通过本次实验，我不仅学到了如何从零开始构建一个简单的操作系统，还深入理解了Linux内核的结构、编译和调试过程。我掌握了内核开发的核心技能，包括环境搭建、内核编译、启动过程、以及如何集成工具集和调试程序。此外，通过与Busybox和initramfs的结合，我更清楚地认识到操作系统的模块化结构，以及如何在不同的硬件环境中进行优化和调整。

