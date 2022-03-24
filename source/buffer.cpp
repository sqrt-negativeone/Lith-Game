
internal Buffer
InitBuffer(M_Arena *arena, u32 buffer_size)
{
    Buffer result = {};
    Assert(arena);
    
    void *data = M_ArenaPushZero(arena, buffer_size);
    if (data)
    {
        result.max_size = buffer_size;
        result.content.data = data;
    }
    return result;
}

internal b32
IsBufferFull(Buffer *buffer)
{
    Assert(buffer);
    
    b32 result = (buffer->max_size == buffer->content.size);
    return result;
}

internal b32
IsBufferEmpty(Buffer *buffer)
{
    Assert(buffer);
    
    b32 result = (buffer->content.size == 0);
    return result;
}

inline void
InsertCharacterToBuffer(Buffer *buffer, char c)
{
    Assert(buffer);
    
    u64 len = buffer->content.size;
    buffer->content.str[len] = c;
    buffer->content.size = len + 1;
}

inline void
RemoveLastCharacterFromBuffer(Buffer *buffer)
{
    Assert(buffer);
    u64 len = buffer->content.size;
    --buffer->content.size;
}

internal void
EmptyBuffer(Buffer *buffer)
{
    Assert(buffer);
    buffer->content.size = 0;
}

internal void
CopyStringToBuffer(Buffer *buffer, s8 string)
{
    Assert(string.size <= buffer->max_size);
    if (string.size <= buffer->max_size)
    {
        MemoryCopy(buffer->content.data, string.data, string.size);
        buffer->content.size = string.size;
    }
}