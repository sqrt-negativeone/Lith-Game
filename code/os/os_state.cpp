
internal void
OS_EndFrame()
{
    ++os->frame_index;
    os->time.game_time += os->time.game_dt_for_frame;
    OS_EatAllEvents(&os->events);
}

internal M_Arena *
OS_FrameArena()
{
    u64 arena_index = os->frame_index % ArrayCount(os->frame_arenas);
    M_Arena *result = os->frame_arenas[arena_index];
    return result;
}


internal b32
IsIdentifierCharacter(u8 character)
{
    b32 Result = !(character == ' ' || character == ',' || character == '\n' || character == '=' || character == '\t' || character == '}');
    return Result;
}

internal Config_File
OS_LoadConfigFile(M_Arena *arena)
{
    Config_File result = {};
    M_Temp scratch = GetScratch(&arena, 1);
    String8 file_content = os->LoadEntireFile(scratch.arena, Str8Lit("data/lith.conf"));
    if (file_content.str)
    {
        for (;;)
        {
            // NOTE(fakhri): name
            u32 start_index = 0;
            while(start_index < file_content.size &&
                  !IsIdentifierCharacter(file_content.str[start_index]))
            {
                start_index++;
            }
            
            u32 end_index = start_index;
            while(end_index < file_content.size &&
                  IsIdentifierCharacter(file_content.str[end_index]))
            {
                end_index++;
            }
            
            if (end_index >= file_content.size)
            {
                break;
            }
            
            String8 name = Substr8(file_content, start_index, end_index);
            
            if (Str8Match(name, Str8Lit("lobby_address"), MatchFlag_CaseInsensitive))
            {
                // NOTE(fakhri): values
                start_index = end_index;
                while(start_index < file_content.size &&
                      !IsIdentifierCharacter(file_content.str[start_index]))
                {
                    start_index++;
                }
                
                if (start_index >= file_content.size || file_content.str[start_index] != '{')
                {
                    break;
                }
                
                for (;file_content.str[start_index] != '}';)
                {
                    start_index++;
                    while(start_index < file_content.size &&
                          !IsIdentifierCharacter(file_content.str[start_index]))
                    {
                        start_index++;
                    }
                    
                    end_index = start_index;
                    while(end_index < file_content.size &&
                          IsIdentifierCharacter(file_content.str[end_index]))
                    {
                        end_index++;
                    }
                    
                    if (end_index == file_content.size)
                    {
                        break;
                    }
                    
                    String8 value = PushStr8Copy(arena, Substr8(file_content, start_index, end_index));
                    Str8ListPush(arena, &result.lobby_addresses, value);
                    start_index = end_index;
                }
                start_index++;
                file_content = Str8Skip(file_content, start_index);
            }
        }
    }
    else
    {
        Log("Couldn't Load Config File");
    }
    return result;
}