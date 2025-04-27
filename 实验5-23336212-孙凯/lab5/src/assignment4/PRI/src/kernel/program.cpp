#include "program.h"
#include "stdlib.h"
#include "interrupt.h"
#include "asm_utils.h"
#include "stdio.h"
#include "thread.h"
#include "os_modules.h"
#include<ctime>

const int PCB_SIZE = 4096;                   // PCB的大小，4KB。
char PCB_SET[PCB_SIZE * MAX_PROGRAM_AMOUNT]; // 存放PCB的数组，预留了MAX_PROGRAM_AMOUNT个PCB的大小空间。
bool PCB_SET_STATUS[MAX_PROGRAM_AMOUNT];     // PCB的分配状态，true表示已经分配，false表示未分配。
int MAX_PRIORITY = 10; // 最大优先级
// 临时使用静态递增计数器
static unsigned long time_counter = 0;
ProgramManager::ProgramManager()
{
    initialize();
}

void ProgramManager::initialize()
{
    allPrograms.initialize();
    readyPrograms.initialize();
    running = nullptr;

    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        PCB_SET_STATUS[i] = false;
    }
}

int ProgramManager::executeThread(ThreadFunction function, void *parameter, const char *name, int priority,int schedulingPolicy)
{
    if (priority < 1 || priority > MAX_PRIORITY)
    {
        return -1;
    }

    // 检查线程名是否重复 - 自己实现字符串比较
    for (ListItem *item = allPrograms.front(); item; item = item->next)
    {
        PCB *program = ListItem2PCB(item, tagInAllList);
        
        // 自己实现字符串比较
        bool isNameSame = true;
        int i = 0;
        
        // 逐字符比较直到结束
        while (i < MAX_PROGRAM_NAME) {
            // 如果当前字符不同，则名称不同
            if (program->name[i] != name[i]) {
                isNameSame = false;
                break;
            }
            
            // 如果到达字符串结尾（两者同时结束），则名称相同
            if (program->name[i] == '\0') {
                break;
            }
            
            // 继续比较下一个字符
            i++;
        }
        
        // 如果名称相同，返回错误
        if (isNameSame) {
            return -1;
        }
    }

    // 检查线程数量是否超过限制
    if (allPrograms.size() >= MAX_PROGRAM_AMOUNT)
    {
        return -1;
    }

        

    // 关中断，防止创建线程的过程被打断
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();
    
    // 分配一页作为PCB
    PCB *thread = allocatePCB();

    if (!thread)
        return -1;

    // 初始化分配的页
    memset(thread, 0, PCB_SIZE);

    for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i)
    {
        thread->name[i] = name[i];
    }

    thread->status = ProgramStatus::READY;
    thread->priority = priority;
    thread->ticks = priority * 10;
    thread->ticksPassedBy = 0;
    thread->pid = ((int)thread - (int)PCB_SET) / PCB_SIZE;
    thread->createTime = time_counter++;
    thread->waitTime = 0;
    // 设置线程的调度策略
    // 0:RR, 1:SJF, 2:PRIORITY, 3:FCFS
    if(schedulingPolicy == 0)
        thread->schedulingPolicy = SchedulingPolicy::SCHEDULED_RR;
    else if(schedulingPolicy == 1)
        thread->schedulingPolicy = SchedulingPolicy::SCHEDULED_SJF;
    else if(schedulingPolicy == 2)
        thread->schedulingPolicy = SchedulingPolicy::SCHEDULED_PRIORITY;
    else if(schedulingPolicy == 3)
        thread->schedulingPolicy = SchedulingPolicy::SCHEDULED_FCFS;
    else
        thread->schedulingPolicy = SchedulingPolicy::SCHEDULED_RR;
    
    // 线程栈
    thread->stack = (int *)((int)thread + PCB_SIZE);
    thread->stack -= 7;
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)function;
    thread->stack[5] = (int)program_exit;
    thread->stack[6] = (int)parameter;

    allPrograms.push_back(&(thread->tagInAllList));
    readyPrograms.push_back(&(thread->tagInGeneralList));

    // 恢复中断
    interruptManager.setInterruptStatus(status);

    return thread->pid;
}

void ProgramManager::schedule(){
    if(running){
        switch (running->schedulingPolicy)
        {
            case SchedulingPolicy::SCHEDULED_RR:
                schedule_RR();
                break;
            case SchedulingPolicy::SCHEDULED_SJF:
                schedule_SJF();
                break;
            case SchedulingPolicy::SCHEDULED_PRIORITY:
                schedule_PRI();
                break;
            case SchedulingPolicy::SCHEDULED_FCFS:
                schedule_FCFS();
                break;
            default:
                break;
        }
    }
}

// 轮转调度
void ProgramManager::schedule_RR()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        running->ticks = running->priority * 10;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }
    else if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    next->status = ProgramStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();
    // 显示进程切换信息
    //printf("pid %d is stop, and pid %d is continue...\n", cur->pid, next->pid);
    asm_switch_thread(cur, next);

    interruptManager.setInterruptStatus(status);
}

//先进先出调度
void ProgramManager::schedule_FCFS()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }
    else if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    next->status = ProgramStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();
    // 显示进程切换信息
    //printf("pid %d is stop, and pid %d is continue...\n", cur->pid, next->pid);
    asm_switch_thread(cur, next);

    interruptManager.setInterruptStatus(status);
}
//短作业优先调度
void ProgramManager::schedule_SJF()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    // 如果当前线程仍在运行，且不是因为时间片耗尽被调度的
    // SJF是非抢占式的，应该让当前线程继续执行，除非它主动放弃CPU或结束
    if (running->status == ProgramStatus::RUNNING && running->ticks > 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    // 处理当前线程状态
    if (running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }
    else if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

    // 找到估计运行时间最短的线程
    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    // 使用更合适的指标来评估作业长度
    // 这里使用ticksPassedBy来预测未来执行时间（已执行时间越短，预期剩余时间可能越长）
    // 或者使用另一个专门的评估值
    for (ListItem *i = readyPrograms.front(); i; i = i->next)
    {
        PCB *program = ListItem2PCB(i, tagInGeneralList);
        
        // 主要排序条件：预期总执行时间（这里用priority*10-ticksPassedBy估计）
        int current_estimate = next->priority * 10 - next->ticksPassedBy;
        int program_estimate = program->priority * 10 - program->ticksPassedBy;
        
        if (program_estimate < current_estimate ||
            (program_estimate == current_estimate && program->createTime < next->createTime)) 
        {
            // 如果预期执行时间相同，选择先创建的
            next = program;
        }
    }

    next->status = ProgramStatus::RUNNING;
    running = next;
    
    // 显示进程切换信息
    /*printf("pid %d pid %d (lass time: %d)\n", 
           cur->pid, next->pid, 
           next->priority * 10 - next->ticksPassedBy);*/
    
    readyPrograms.erase(&(next->tagInGeneralList));
    
    asm_switch_thread(cur, next);
    
    // 恢复中断状态
    interruptManager.setInterruptStatus(status);
}

//优先级调度
void ProgramManager::schedule_PRI()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }
    else if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;

    // 找到优先级最高的进程
    for (ListItem *i = readyPrograms.front(); i; i = i->next)
    {
        PCB *program = ListItem2PCB(i, tagInGeneralList);
        if (program->priority > next->priority||(program->priority==next->priority&&program->createTime<next->createTime))
        {
            next = program;
        }
    }

    next->status = ProgramStatus::RUNNING;
    running = next;

    // 显示进程切换信息
    
    readyPrograms.erase(&(next->tagInGeneralList));

    asm_switch_thread(cur, next);

    interruptManager.setInterruptStatus(status);
}

void program_exit()
{
    PCB *thread = programManager.running;
    thread->status = ProgramStatus::DEAD;

    if (thread->pid)
    {
        programManager.schedule();
    }
    else
    {
        interruptManager.disableInterrupt();
        printf("halt\n");
        asm_halt();
    }
}

PCB *ProgramManager::allocatePCB()
{
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        if (!PCB_SET_STATUS[i])
        {
            PCB_SET_STATUS[i] = true;
            return (PCB *)((int)PCB_SET + PCB_SIZE * i);
        }
    }

    return nullptr;
}

void ProgramManager::releasePCB(PCB *program)
{
    int index = ((int)program - (int)PCB_SET) / PCB_SIZE;
    PCB_SET_STATUS[index] = false;
}