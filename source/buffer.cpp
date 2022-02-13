
internal Buffer
InitBuffer(M_Arena *arena, u32 buffer_size)
{
    Buffer result = {};
    Assert(arena);
    
    void *data = M_ArenaPushZero(arena, buffer_size);
    if (data)
    {
        result.max_size = buffer_size;
        result.buffer.data = data;
    }
    return result;
}

internal b32
IsBufferFull(Buffer *buffer)
{
    Assert(buffer);
    
    b32 result = (buffer->max_size == buffer->buffer.size);
    return result;
}

internal b32
IsBufferEmpty(Buffer *buffer)
{
    Assert(buffer);
    
    b32 result = (buffer->buffer.size == 0);
    return result;
}

inline void
InsertCharacterToBuffer(Buffer *buffer, char c)
{
    Assert(buffer);
    
    u64 len = buffer->buffer.size;
    buffer->buffer.str[len] = c;
    buffer->buffer.size = len + 1;
}

inline void
RemoveLastCharacterFromBuffer(Buffer *buffer)
{
    Assert(buffer);
    u64 len = buffer->buffer.size;
    --buffer->buffer.size;
}

internal void
EmptyBuffer(Buffer *buffer)
{
    Assert(buffer);
    buffer->buffer.size = 0;
}