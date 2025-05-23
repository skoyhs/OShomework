#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;

void first_thread(void *arg)
{   
    printf("23336212_sk test physical memory manager\n");
    //测试1：基本分配和释放
    printf("test 1: allocate and release\n");
    int page1 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 1);
    if(page1){
        printf("allocate 1 page success: 0x%x\n", page1);
        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, page1, 1);
        printf("release 1 page success: 0x%x\n", page1);
        int page1_again = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 1);
        if(page1==page1_again){
            printf("allocate 1 page again success: page1==page1_again 0x%x\n", page1_again);
        }else{
            printf("allocate 1 page again failed, new address: 0x%x\n", page1_again);
        }
        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, page1_again, 1);
    }else{
        printf("allocate 1 page failed\n");
    }

    //测试2：连续多页分配
    printf("test 2: allocate and release continuous pages\n");
    int page2 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 2);
    int page2_1 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 1);
    if(page2&&page2_1){
        printf("allocate 2 pages success: 0x%x\n", page2);
        printf("allocate 1 page success: 0x%x\n", page2_1);
        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, page2, 2);
        printf("release 2 pages success: 0x%x\n", page2);
    }else{
        printf("allocate 2 pages failed\n");
    }

    int page2_again = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 2);
    if(page2==page2_again){
        printf("allocate 2 pages again success: page2==page2_again 0x%x\n", page2_again);
        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, page2_again, 2);
    }else{
        printf("allocate 2 pages again failed, new address: 0x%x\n", page2_again);
    }
    memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, page2_1, 1);

    

    //测试3：用户内存分配与释放
    printf("test 3: allocate and release user pages\n");
    int page3 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 4);
    if(page3){
        printf("allocate 4 user pages success: 0x%x\n", page3);
        memoryManager.releasePhysicalPages(AddressPoolType::USER, page3, 4);
        printf("release 4 user pages success: 0x%x\n", page3);
    }else{
        printf("allocate 4 user pages failed\n");
    }

    //测试4：分配超过最大值
    printf("test 4: allocate more than max\n");
    int page4 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 16000);
    if(page4){
        printf("allocate 16000 pages success: 0x%x\n", page4);
        memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, page4, 16000);
        printf("release 16000 pages success: 0x%x\n", page4);
    }else{
        printf("allocate 16000 pages failed\n");
    }
    printf("test done\n");
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
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
