/* date = February 19th 2022 2:16 pm */

#ifndef WIN32_NETWORK_THREAD_H
#define WIN32_NETWORK_THREAD_H

#define HOST_PORT "23451"
#define LOBBY_ADDRESS "127.0.0.1"
#define LOBBY_PLAYER_PORT "42069"
#define LOBBY_SERVER_PORT "12345"
#define SERVER_PORT "42069"

// IMPORTANT(fakhri): THIS IMPLEMENTATION OF CIRCULAR BUFFER ASSUMES 
// WE HAVE SINGLE PRODUCER/SINGLE CONSUMER
struct MessageQueue
{
    Message queue[256];
    u64 head;
    u64 tail;
};

// NOTE(fakhri): messages coming from the player
global MessageQueue player_message_queue;
// NOTE(fakhri): messages coming from the host
global MessageQueue host_message_queue;
global HANDLE network_thread_iocp_handle;

typedef u8 NetworkMessageSource;
enum
{
    NetworkMessageSource_Player,
    NetworkMessageSource_Host,
};


internal inline b32
W32_IsMessageQueueEmpty(MessageQueue *queue)
{
    b32 result = (queue->tail == queue->head);
    return result;
}

// NOTE(fakhri): queue reading operation
internal Message *
W32_BeginMessageQueueRead(MessageQueue *message_queue)
{
    u64 message_index = message_queue->head % ArrayCount(message_queue->queue);
    Message *result = message_queue->queue + message_index;
    return result;
}

internal void
W32_EndMessageQueueRead(MessageQueue *message_queue)
{
    MemoryBarrier();
    message_queue->head++;
}

// NOTE(fakhri): queue writing operation
internal Message *
W32_BeginMessageQueueWrite(MessageQueue *message_queue)
{
    Assert(message_queue->tail - message_queue->head < ArrayCount(message_queue->queue));
    u64 message_index = message_queue->tail % ArrayCount(message_queue->queue);
    Message *result = message_queue->queue + message_index;
    return result;
}

internal void
W32_EndMessageQueueWrite(MessageQueue *message_queue)
{
    MemoryBarrier();
    message_queue->tail++;
}

#endif //WIN32_NETWORK_THREAD_H
