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
//分别对应烟草，纸和胶水
Semaphore tobacco,paper,glue;
//完成信号
Semaphore done; 


void supplier(void *arg){
    int flag=0;
    while(1){
        //随机产生组合
        done.P();
        flag=(flag+1)%3;
        switch(flag){
            case 0:
                tobacco.V();
                stdio.print("supplier: paper and glue! next thread should be smoker_1!\n");
                break;
            case 1:
                paper.V();
                stdio.print("supplier: tobacco and glue! next thread should be smoker_2!\n");
                break;
            case 2:
                glue.V();
                stdio.print("supplier: tobacco and paper! next thread should be smoker_3!\n");
                break;
        }
        int delay=0xfffffff;
        while(delay--);
      
    
    }
}


void smoker_1(void *arg){
    while(1){
        tobacco.P();
        printf("smoker_1 6212: I have paper and glue! I am smoking!\n");
        done.V();
    }
}

void smoker_2(void *arg){
    while(1){
        paper.P();
        printf("smoker_2: I have tobacco and glue! I am smoking!\n");
        done.V();
    }
}

void smoker_3(void *arg){
    
    while(1){
        glue.P();
        printf("smoker_3: I have tobacco and paper! I am smoking!\n");
        done.V();
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
    done.initialize(1);
    tobacco.initialize(0);
    paper.initialize(0);
    glue.initialize(0);
    // 创建生产者线程
    programManager.executeThread(supplier, nullptr, "second thread: supplier", 1);
    // 创建消费者线程
    programManager.executeThread(smoker_1, nullptr, "third thread: smoker_1", 1);
    programManager.executeThread(smoker_2, nullptr, "forth thread: smoker_2", 1);
    programManager.executeThread(smoker_3, nullptr, "fifth thread: smoker_3", 1);

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
