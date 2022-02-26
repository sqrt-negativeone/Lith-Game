/* date = February 19th 2022 2:16 pm */

#ifndef WIN32_NETWORK_THREAD_H
#define WIN32_NETWORK_THREAD_H

#define LOBBY_ADDRESS "127.0.0.1"
#define LOBBY_PORT "42069"
#define SERVER_PORT "42069"

struct MessageQueue
{
    NetworkMessage queue[256];
    u64 head;
    u64 tail;
};

// NOTE(fakhri): the messages that we send to the server are queued here
global MessageQueue global_network_sending_queue;
// NOTE(fakhri): the messages that we receive from the server are queued here
global MessageQueue global_network_receiving_queue;

global HANDLE global_iocp_handle;

#define GAME_MESSAGE   0
#define SERVER_MESSAGE 1

internal void
W32_PushNetworkMessage(MessageQueue *message_queue, NetworkMessage msg)
{
    u64 message_index = message_queue->tail;
    message_queue->queue[message_index] = msg;
    MemoryBarrier();
    ++message_queue->tail;
}

internal void
W32_PushNetworkMessageToServer(NetworkMessage msg)
{
    W32_PushNetworkMessage(&global_network_sending_queue, msg);
    // NOTE(fakhri): notify the network thread
    u64 completion_key = GAME_MESSAGE;
    PostQueuedCompletionStatus(global_iocp_handle, 0, completion_key, 0);
}

internal void
W32_PushNetworkMessageToPlayer(NetworkMessage msg)
{
    W32_PushNetworkMessage(&global_network_receiving_queue, msg);
}

internal b32
W32_IsMessageQueueEmpty(MessageQueue *queue)
{
    b32 result = (queue->tail == queue->head);
    return result;
}

internal NetworkMessageResult
W32_GetNextMessageIfAvailable(MessageQueue *message_queue)
{
    NetworkMessageResult result = {};
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

internal NetworkMessageResult
W32_GetNextNetworkMessageIfAvailable()
{
    NetworkMessageResult result = W32_GetNextMessageIfAvailable(&global_network_receiving_queue);
    return result;
}

#endif //WIN32_NETWORK_THREAD_H
