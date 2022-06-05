/* date = June 5th 2022 4:06 pm */

#ifndef OS_WORK_QUEUE_H
#define OS_WORK_QUEUE_H

typedef void OS_WorkThreadWork(void *);

struct OS_WorkQueue_Entry
{
    OS_WorkThreadWork *work;
    void *data;
};

struct OS_WorkQueue
{
    OS_WorkQueue_Entry queue[512];
    u64 head;
    u64 tail;
    
    Semaphore_Handle waiting_worker_threads_semaphore;
    Mutex_Handle     producer_mutex;
};

struct OS_PopQueueResult
{
    OS_WorkQueue_Entry work_entry;
    b32 valid;
};

#endif //OS_WORK_QUEUE_H
