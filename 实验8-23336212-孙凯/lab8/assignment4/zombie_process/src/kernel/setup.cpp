#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"
#include "syscall.h"
#include "tss.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;
// 系统调用
SystemService systemService;
// Task State Segment
TSS tss;

int syscall_0(int first, int second, int third, int forth, int fifth)
{
    printf("systerm call 0: %d, %d, %d, %d, %d\n",
           first, second, third, forth, fifth);
    return first + second + third + forth + fifth;
}

// init进程，回收所有僵尸进程
void init_process(void *arg)
{
    printf("Init process started (PID=1), waiting for zombie processes\n");
    
    int retval;
    while (1)
    {
        // 尝试回收所有子进程
        int pid;
        while ((pid = wait(&retval)) != -1)
        {
            printf("Init: collected zombie process PID=%d (retval=%d)\n", 
                   pid, retval);
        }
        
        // 延时一段时间
        uint32 tmp = 0xffffff;
        while (tmp) --tmp;
    }
}

void first_process()
{
    int pid = fork();
    int retval;

    if (pid)
    {
        pid = fork();
        if (pid)
        {
            //父进程直接退出
            printf("parent process exit, pid: %d\n", programManager.running->pid);
            asm_halt();
        }
        else
        {
            uint32 tmp = 0xffffff;
            while (tmp)
                --tmp;
            printf("exit, pid: %d\n", programManager.running->pid);
            exit(123934);
        }
    }
    else
    {
        uint32 tmp = 0xffffff;
        while (tmp)
            --tmp;
        printf("exit, pid: %d\n", programManager.running->pid);
        exit(-123);
    }
}

//添加一个监控僵尸进程的线程
void monitor_zombie_thread(void *arg)
{
    while(true)
    { 
        //延迟
        uint tmp = 0xffffffff;
        while (tmp)
            --tmp;
       programManager.showZombieProcesses();
    }
}

void second_thread(void *arg)
{
    printf("thread exit\n");
    exit(0);
}

void first_thread(void *arg)
{

    printf("start process\n");

    // 首先创建init进程（确保pid为1）
    int initPid = programManager.executeThread(init_process, nullptr, "init", 1);
    printf("Created init process with PID: %d\n", initPid);
    // 创建一个监控僵尸进程的线程
    programManager.executeThread(monitor_zombie_thread, nullptr, "monitor zombie", 1);
    programManager.executeProcess((const char *)first_process, 1);
    //programManager.executeThread(second_thread, nullptr, "second", 1);
    asm_halt();
}

extern "C" void setup_kernel()
{

    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);

    // 输出管理器
    stdio.initialize();

    // 进程/线程管理器
    programManager.initialize();

    // 初始化系统调用
    systemService.initialize();
    // 设置0号系统调用
    systemService.setSystemCall(0, (int)syscall_0);
    // 设置1号系统调用
    systemService.setSystemCall(1, (int)syscall_write);
    // 设置2号系统调用
    systemService.setSystemCall(2, (int)syscall_fork);
    // 设置3号系统调用
    systemService.setSystemCall(3, (int)syscall_exit);
    // 设置4号系统调用
    systemService.setSystemCall(4, (int)syscall_wait);

    // 内存管理器
    memoryManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = ProgramStatus::RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
