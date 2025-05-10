#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

Semaphore chopstick[5];

void philosopher(void *arg)
{
    int id = (int)arg;
    while (1)
    {
        // 思考
        if(id==0){
            printf("philosopher 6212 %d is thinking\n", id);
            int delay=0xfffffff;
            while (delay--);

            // 尝试拿起筷子
            chopstick[id].P();
            printf("philosopher 6212 %d picked up chopstick %d\n", id, id);
            chopstick[(id + 1) % 5].P();
            printf("philosopher 6212 %d picked up chopstick %d\n", id, (id + 1) % 5);

            // 吃饭
            printf("philosopher 6212 %d is eating\n", id);
            delay=0xfffffff;
            while (delay--);

            // 放下筷子
            chopstick[id].V();
            printf("philosopher 6212 %d put down chopstick %d\n", id, id);
            chopstick[(id + 1) % 5].V();
            printf("philosopher 6212 %d put down chopstick %d\n", id, (id + 1) % 5);
            delay=0xfffffff;
            while (delay--);
        }
        else{
            printf("philosopher %d is thinking\n", id);
            int delay=0xfffffff;
            while (delay--);

            // 尝试拿起筷子
            chopstick[id].P();
            printf("philosopher %d picked up chopstick %d\n", id, id);
            chopstick[(id + 1) % 5].P();
            printf("philosopher %d picked up chopstick %d\n", id, (id + 1) % 5);

            // 吃饭
            printf("philosopher %d is eating\n", id);
            delay=0xfffffff;
            while (delay--);

            // 放下筷子
            chopstick[id].V();
            printf("philosopher %d put down chopstick %d\n", id, id);
            chopstick[(id + 1) % 5].V();
            printf("philosopher %d put down chopstick %d\n", id, (id + 1) % 5);
            delay=0xfffffff;
            while (delay--);
        }
        
    }
}
void first_thread(void *arg)
{
    // 第1个线程不可以返回
    stdio.moveCursor(0);
    for (int i = 0; i < 25 * 80; ++i)
    {
        stdio.print(' ');
    }
    stdio.moveCursor(0);
    for (int i=0;i<5;i++){
        chopstick[i].initialize(1);
    }
    // 创建哲学家线程
    programManager.executeThread(philosopher, (void*)0, "second thread: philosopher_0", 1);
    programManager.executeThread(philosopher, (void*)1, "third thread: philosopher_1", 1);
    programManager.executeThread(philosopher, (void*)2, "forth thread: philosopher_2", 1);
    programManager.executeThread(philosopher, (void*)3, "fifth thread: philosopher_3", 1);
    programManager.executeThread(philosopher, (void*)4, "sixth thread: philosopher_4", 1);


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
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
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
