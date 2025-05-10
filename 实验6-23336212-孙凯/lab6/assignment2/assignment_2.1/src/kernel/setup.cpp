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
/* Semaphore tobacco,paper,glue;
Semaphore done; */
 int tobacco=0,paper=0,glue=0;
 int done=1;

void supplier(void *arg){
    int flag=0;
    while(1){
    while(done){
        //随机产生组合
        flag=(flag+1)%3;
        printf("Now: tobacco: %d, paper: %d, glue: %d\n", tobacco, paper, glue);
        switch(flag){
            case 0:
                paper++;
                glue++;
                stdio.print("supplier: paper and glue! next thread should be smoker_1!\n");
                break;
            case 1:
                tobacco++;
                glue++;
                stdio.print("supplier: tobacco and glue! next thread should be smoker_2!\n");
                break;
            case 2:
                tobacco++;
                paper++;
                stdio.print("supplier: tobacco and paper! next thread should be smoker_3!\n");
                break;
        }
        done=0;
      
    }
    }
}


void smoker_1(void *arg){
    bool paper_flag=false;
    bool glue_flag=false;
    while(1){
        if(paper){
            paper_flag=true;
        }
        if(glue){
            glue_flag=true;
            //printf("smoker_1: I have glue! Now: tobacco: %d, paper: %d, glue: %d\n",tobacco, paper, glue);
        }
        if(paper_flag && glue_flag){
            printf("smoker_1 6212: I have paper and glue! Now: tobacco: %d, paper: %d, glue: %d\n",tobacco, paper, glue);

            paper_flag=false;
            glue_flag=false;
            paper--;
            glue--;
            done=1;
            //tobacco--;

        }
        
    }
}

void smoker_2(void *arg){
    bool tobacco_flag=false;
    bool glue_flag=false;
    while(1){
        if(tobacco){
            tobacco_flag=true;
            
            //printf("smoker_2: I have tobacco! Now: tobacco: %d, paper: %d, glue: %d\n",tobacco, paper, glue);
        }
        if(glue){
            glue_flag=true;
          
            //printf("smoker_2: I have glue! Now: tobacco: %d, paper: %d, glue: %d\n",tobacco, paper, glue);
        }
        if(tobacco_flag && glue_flag){
            printf("smoker_2: I have tobacco and glue! Now: tobacco: %d, paper: %d, glue: %d\n",tobacco, paper, glue);
            tobacco_flag=false;
            glue_flag=false;
            glue--;
            tobacco--;
            done=1;
           
        }
        
    }
}

void smoker_3(void *arg){
    bool tobacco_flag=false;
    bool paper_flag=false;
    while(1){
        if(tobacco){
            tobacco_flag=true;
            //printf("smoker_3: I have tobacco! Now: tobacco: %d, paper: %d, glue: %d\n",tobacco, paper, glue);
        }
        if(paper){
            paper_flag=true;
            //printf("smoker_3: I have paper! Now: tobacco: %d, paper: %d, glue: %d\n",tobacco, paper, glue);
        }
        if(tobacco_flag && paper_flag){
            printf("smoker_3: I have tobacco and paper! Now: tobacco: %d, paper: %d, glue: %d\n",tobacco, paper, glue);
            tobacco_flag=false;
            paper_flag=false;
            tobacco--;
            paper--;
            done=1;
            //glue--;
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
