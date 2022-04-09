/* date = February 19th 2022 2:16 pm */

#ifndef WIN32_NETWORK_THREAD_H
#define WIN32_NETWORK_THREAD_H

#define HOST_PORT "23451"
#define LOBBY_ADDRESS "127.0.0.1"
#define LOBBY_PLAYER_PORT "42069"
#define LOBBY_SERVER_PORT "12345"
#define SERVER_PORT "42069"

struct MessageQueue
{
    Message queue[256];
    u64 head;
    u64 tail;
};

// NOTE(fakhri): the messages that we send to the server are queued here
global MessageQueue network_sending_queue;
// NOTE(fakhri): the messages that we receive from the server are queued here
global MessageQueue network_receiving_queue;
global HANDLE network_thread_iocp_handle;

#define GAME_MESSAGE   0
#define SERVER_MESSAGE 1

internal void
W32_PushMessageQueue(MessageQueue *message_queue, Message msg)
{
    u64 message_index = message_queue->tail;
    message_queue->queue[message_index] = msg;
    MemoryBarrier();
    ++message_queue->tail;
}

internal void
W32_PushNetworkMessageToServer(Message msg)
{
    W32_PushMessageQueue(&network_sending_queue, msg);
    // NOTE(fakhri): notify the network thread
    u64 completion_key = GAME_MESSAGE;
    PostQueuedCompletionStatus(network_thread_iocp_handle, 0, completion_key, 0);
}

internal void
W32_PushNetworkMessageToPlayer(Message msg)
{
    W32_PushMessageQueue(&network_receiving_queue, msg);
}

internal b32
W32_IsMessageQueueEmpty(MessageQueue *queue)
{
    b32 result = (queue->tail == queue->head);
    return result;
}

internal MessageResult
W32_GetNextMessageIfAvailable(MessageQueue *message_queue)
{
    MessageResult result = {};
    Assert(message_queue);
    if (!W32_IsMessageQueueEmpty(message_queue))
    {
        // NOTE(fakhri): single consumer single producer
        u64 message_index = message_queue->head % ArrayCount(message_queue->queue);
        result.message = message_queue->queue[message_index];
        MemoryBarrier();
        message_queue->head++;
        result.is_available = 1;
    }
    else
    {
        result.is_available = 0;
    }
    return result;
}

internal MessageResult
W32_GetNextNetworkMessageIfAvailable()
{
    MessageResult result = W32_GetNextMessageIfAvailable(&network_receiving_queue);
    return result;
}

#endif //WIN32_NETWORK_THREAD_H
