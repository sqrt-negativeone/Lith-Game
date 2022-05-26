/* date = May 22nd 2022 9:00 pm */

#ifndef WIN32_WORK_THREADS_H
#define WIN32_WORK_THREADS_H

#define InvalidSocket INVALID_SOCKET
typedef HANDLE Semaphore_Handle;
typedef HANDLE Mutex_Handle;
typedef HANDLE Thread_Handle;



typedef void WorkThreadWork(void *);

struct Queue_Entry
{
    WorkThreadWork *work;
    void *data;
};

struct Work_Queue
{
    Queue_Entry queue[512];
    u64 head;
    u64 tail;
    
    Semaphore_Handle waiting_worker_threads_semaphore;
    Mutex_Handle     producer_mutex;
};

struct PopQueueResult
{
    Queue_Entry work_entry;
    b32 valid;
};


internal b32 W32_ProcessOneWorkQueueEntry();
internal b32 W32_PushWorkQueueEntrySP(WorkThreadWork *work, void *data);
internal b32 W32_PushWorkQueueEntry(WorkThreadWork *work, void *data);

global Work_Queue w32_work_queue;
#endif //WIN32_WORK_THREADS_H
