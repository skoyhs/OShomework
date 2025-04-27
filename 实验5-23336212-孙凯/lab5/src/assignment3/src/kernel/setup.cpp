#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 添加一个辅助函数，将调度策略枚举转换为字符串
const char* get_scheduling_policy_name(enum SchedulingPolicy policy) {
    switch (policy) {
        case SCHEDULED_RR:
            return "Round Robin";
        case SCHEDULED_SJF:
            return "Shortest Job First";
        case SCHEDULED_PRIORITY:
            return "Priority";
        case SCHEDULED_FCFS:
            return "First Come First Served";
        default:
            return "Unknown";
    }
}

void third_thread(void *arg) {
    printf("pid %d name \"%s\", scheduling policy: %s ,creatTime: %d :Hello World!\n", 
           programManager.running->pid, 
           programManager.running->name, 
           get_scheduling_policy_name(programManager.running->schedulingPolicy),
           programManager.running->createTime);
          while(1){

          }
    
}

void second_thread(void *arg) {
    printf("pid %d name \"%s\", scheduling policy: %s ,creatTime: %d :Hello World!\n", 
           programManager.running->pid, 
           programManager.running->name, 
           get_scheduling_policy_name(programManager.running->schedulingPolicy),
           programManager.running->createTime);
    asm_halt();
}

void first_thread(void *arg)
{
    // 第1个线程不可以返回
    printf("pid %d name \"%s\", scheduling policy: %s ,creatTime: %d :Hello World!\n", 
           programManager.running->pid, 
           programManager.running->name, 
           get_scheduling_policy_name(programManager.running->schedulingPolicy));
    if (programManager.running->pid==0)
    {
        programManager.executeThread(second_thread, nullptr, "second thread", 2, 0);
        programManager.executeThread(third_thread, nullptr, "third thread", 3, 0);
    }
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

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1,0);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
