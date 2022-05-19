
internal Game_Command *
GameCommand_PushCommand(Game_Command_Buffer *command_buffer)
{
    Game_Command *game_command = 0;
    if (command_buffer->free_list)
    {
        game_command = command_buffer->free_list;
        command_buffer->free_list = game_command->next;
        MemoryZero(game_command, sizeof(Game_Command));
    }
    
    if (!game_command)
    {
        game_command = PushStructZero(command_buffer->arena, Game_Command);
    }
    
    if (!command_buffer->first)
    {
        command_buffer->first = command_buffer->last = game_command;
    }
    else
    {
        command_buffer->last->next = game_command;
        command_buffer->last       = game_command;
    }
    return game_command;
}

internal Game_Command *
GameCommand_PushCommand_DisplayMessag(Game_Command_Buffer *command_buffer, String8 string, v4 string_color, v3 string_position, Coordinate_Type coords_type, f32 duration)
{
    Game_Command *game_command = GameCommand_PushCommand(command_buffer);
    Assert(game_command);
    Assert(string.len <= ArrayCount(game_command->buffer));
    
    game_command->string.cstr = game_command->buffer;
    MemoryCopy(game_command->string.str, string.str, string.len);
    game_command->string.len = string.len;
    
    game_command->string_position = string_position;
    game_command->kind            = GameCommandKind_DisplayMessage;
    game_command->duration        = duration;
    game_command->string_color    = string_color;
    game_command->coords_type     = coords_type;
    
    return game_command;
}

internal Game_Command *
GameCommand_PushCommand_BurnCards(Game_Command_Buffer *command_buffer)
{
    Game_Command *game_command = GameCommand_PushCommand(command_buffer);
    Assert(game_command);
    
    game_command->kind     = GameCommandKind_BurnCards;
    game_command->duration = 0;
    
    return game_command;
}

internal Game_Command *
GameCommand_PushCommand_OpenDeclareMenu(Game_Command_Buffer *command_buffer)
{
    Game_Command *game_command = GameCommand_PushCommand(command_buffer);
    Assert(game_command);
    {
        
    }
    game_command->kind = GameCommandKind_OpenDeclareMenu;
    
    return game_command;
}

internal Game_Command *
GameCommand_PushCommand_Delay(Game_Command_Buffer *command_buffer, f32 duration)
{
    Game_Command *game_command = GameCommand_PushCommand(command_buffer);
    Assert(game_command);
    
    game_command->kind     = GameCommandKind_Delay;
    game_command->duration = duration;
    
    return game_command;
}

internal void
GameCommand_EatFirstCommand(Game_Command_Buffer *command_buffer)
{
    Assert(command_buffer->first);
    Game_Command *first = command_buffer->first;
    if (first)
    {
        command_buffer->first = first->next;
        first->next = command_buffer->free_list;
        command_buffer->free_list = first;
    }
}
