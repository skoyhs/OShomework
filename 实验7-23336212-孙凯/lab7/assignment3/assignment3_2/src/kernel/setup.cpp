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
    // 测试页目录和页表虚拟地址构造
    // 1. 测试第141个页目录项
    int dir_index_141 = 141;
    int vaddr_141 = dir_index_141 << 22;  // 构造页目录索引为141的虚拟地址
    int pde_addr_141 = memoryManager.toPDE(vaddr_141);
    printf("141 Page Directory VirtualAddress: 0x%x\n", pde_addr_141);
    
    // 理论计算值
    int expected_pde_141 = 0xFA3E8000 + (dir_index_141 * 4);
    printf("Theoretical Address: 0x%x, %s\n", 
           expected_pde_141, 
           pde_addr_141 == expected_pde_141 ? "right" : "fault");
    
    // 2. 测试第891个页目录项指向的页表中第109个页表项
    int dir_index_891 = 891;
    int table_index_109 = 109;
    int vaddr_891_109 = (dir_index_891 << 22) | (table_index_109 << 12);
    int pte_addr_891_109 = memoryManager.toPTE(vaddr_891_109);
    printf("891 Page Directory 109 page table VirtualAddress: 0x%x\n", pte_addr_891_109);
    
    // 理论计算值
    int expected_pte_891_109 = 0xFA000000 + (dir_index_891 << 12) + (table_index_109 * 4);
    printf("Theoretical Address: 0x%x, %s\n", 
           expected_pte_891_109, 
           pte_addr_891_109 == expected_pte_891_109 ? "right" : "fault");
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
    memoryManager.openPageMechanism();


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
