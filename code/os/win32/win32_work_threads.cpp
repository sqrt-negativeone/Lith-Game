

internal b32
W32_PushWorkQueueEntrySP(WorkThreadWork *work, void *data)
{
    // NOTE(fakhri): safe to call this without mutex if single producer
    b32 result = false;
    if (w32_work_queue.tail - w32_work_queue.head < ArrayCount(w32_work_queue.queue))
    {
        result = true;
        u64 entry_index = w32_work_queue.tail % ArrayCount(w32_work_queue.queue);
        w32_work_queue.queue[entry_index].work = work;
        w32_work_queue.queue[entry_index].data = data;
        MemoryBarrier();
        ++w32_work_queue.tail;
        // NOTE(fakhri): wake a sleeping thread if there are any
        ReleaseSemaphore(w32_work_queue.waiting_worker_threads_semaphore, 1, 0);
    }
    return result;
}

internal b32
W32_PushWorkQueueEntry(WorkThreadWork *work, void *data)
{
    b32 result = false;
    
    WaitForSingleObject(w32_work_queue.producer_mutex, INFINITE);
    result = W32_PushWorkQueueEntrySP(work, data);
    ReleaseMutex(w32_work_queue.producer_mutex);
    
    return result;
}

internal inline b32
W32_IsWorkQueueEmpty()
{
    b32 result = (w32_work_queue.tail == w32_work_queue.head);
    return result;
}

internal PopQueueResult
PopQueueEntry()
{
    PopQueueResult result = {};
    u64 old_head = w32_work_queue.head;
    u64 head_index = old_head % ArrayCount(w32_work_queue.queue);
    result.work_entry = w32_work_queue.queue[head_index];
    volatile u64 *destination = (volatile u64*)&w32_work_queue.head;
    if (InterlockedCompareExchange(destination, old_head + 1, old_head) == old_head)
    {
        result.valid = true;
    }
    return result;
}

internal b32
W32_ProcessOneWorkQueueEntry()
{
    b32 result = false;
    while(!W32_IsWorkQueueEmpty())
    {
        PopQueueResult pop_result = PopQueueEntry();
        if (pop_result.valid)
        {
            result = true;
            Queue_Entry work_entry = pop_result.work_entry;
            work_entry.work(work_entry.data);
            break;
        }
    }
    return result;
}

internal DWORD WINAPI 
W32_WorkerThreadMain(LPVOID param)
{
    Work_Queue *work_queue = (Work_Queue *)param;
    
    for(;;)
    {
        b32 did_work = W32_ProcessOneWorkQueueEntry();
        if (!did_work)
        {
            // NOTE(fakhri): queue is empty
            WaitForSingleObject(w32_work_queue.waiting_worker_threads_semaphore, INFINITE);
        }
    }
}
