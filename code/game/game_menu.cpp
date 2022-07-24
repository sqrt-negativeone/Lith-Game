
internal void
StartGame(Game_State *game_state)
{
    SetFlag(game_state->flags, StateFlag_WaitingForCards);
    glDepthFunc(GL_LESS);
}

#define UI_MenuSectionBegin(ui, menu_kind)          \
{                                                 \
Game_Menu *menu = (ui)->menus + (menu_kind);    \
if (UI_MenuSection(ui, menu_kind, dt))
#define UI_MenuSectionEnd()                         \
}

internal void
UI_HostingGameMessage(Game_State *game_state, Game_UI *ui, f32 &x, f32 &y, f32 dt, f32 fade_in)
{
    if (HasFlag(game_state->flags, StateFlag_HostingGame))
    {
        y = 0.3f * game_state->render_context.screen.y;
        UI_Label(ui, x, y, Str8Lit("You are hosting the game"), Vec4(0.8f, 0.8f, 0.8f, 1.0f), fade_in);
        y += .85f * VerticalAdvanceFontHeight(ui);
        
        UI_Label(ui, x, y, Str8Lit("Game ID is"), Vec4(0.8f, 0.8f, 0.8f, 1.0f), fade_in);
        y += 0.85f * VerticalAdvanceFontHeight(ui);
        
        String8 button_text = game_state->game_id;
        b32 accept_input = 1;
        if (game_state->game_id_copie_message_time > 0)
        {
            game_state->game_id_copie_message_time -= dt;
            accept_input = 0;
            button_text = Str8Lit("Game ID Copied to clipboard");
        }
        
        ChangeActiveFont(ui, FontKind_InputField);
        if (UI_Button(ui, x, y, button_text, dt, fade_in, accept_input))
        {
            game_state->game_id_copie_message_time = Seconds(1);
            os->CopyStringToClipboard(game_state->game_id);
        }
        y += VerticalAdvanceFontHeight(ui);
        ChangeActiveFont(ui, FontKind_MenuItem);
    }
}

internal void
GameMenu(Game_State *game_state, f32 dt)
{
    Game_UI *ui = &game_state->ui;
    UI_Begin(ui);
    
    //~ NOTE(fakhri): main menu
    UI_MenuSectionBegin(ui, GameMenuKind_Main)
    {
        f32 fade_in = menu->presence;
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_GameTitle);
        
        UI_Label(ui, x, y, Str8Lit("Lith"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.5f * game_state->render_context.screen.y;
        if (UI_Button(ui, x, y, Str8Lit("Join Game"), dt, fade_in))
        {
            UI_OpenMenu(ui, GameMenuKind_JoinGame);
        }
        y += VerticalAdvanceFontHeight(ui);
        
        if (UI_Button(ui, x, y, Str8Lit("Host Game"), dt, fade_in))
        {
            
            os->StartGameHost();
            os->PushWorkQueueEntry(GameHostWork, 0);
            PushHostGameSessionNetworkMessage();
            UI_OpenMenu(ui, GameMenuKind_HostGame);
        }
        y += VerticalAdvanceFontHeight(ui);
        
        if (UI_Button(ui, x, y, Str8Lit("Exit"), dt, fade_in))
        {
            os->quit = 1;
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_JoinGame)
    {
        f32 fade_in = menu->presence;
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        
        UI_Label(ui, x, y, Str8Lit("Join Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.4f * game_state->render_context.screen.y;
        UI_Label(ui, x, y, Str8Lit("Game ID:"), Vec4(0.8f, 0.8f, 0.8f, 1.0f), fade_in);
        y += VerticalAdvanceFontHeight(ui);
        
        ChangeActiveFont(ui, FontKind_InputField);
        
        UI_InputField(ui, InputFieldKind_GameID, x, y, dt, fade_in);
        y += 1.5f * VerticalAdvanceFontHeight(ui);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        
        if (UI_Button(ui, x, y, Str8Lit("Join"), dt, fade_in))
        {
            Game_UI_InputField *input_field = ui->input_fields + InputFieldKind_GameID;
            if (input_field->size)
            {
                String8 game_id = Str8(input_field->buffer, input_field->size);
                PushJoinGameSessionNetworkMessage(game_id);
                UI_OpenMenu(ui, GameMenuKind_JoinGameConfirmation);
            }
        }
        y += VerticalAdvanceFontHeight(ui);
        
        if (UI_Button(ui, x, y, Str8Lit("Back"), dt, fade_in))
        {
            UI_OpenMenu(ui, GameMenuKind_Main);
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_JoinGameConfirmation)
    {
        f32 fade_in = menu->presence;
        
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        UI_Label(ui, x, y, Str8Lit("Join Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.4f * game_state->render_context.screen.y;
        
        f32 color_change = Square(SinF(os->time.game_time));
        v4 color1 = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        v4 color2 = Vec4(0.8f, 0.8f, 0.8f, 1.0f);
        v4 color = color_change * color1 + (1 - color_change) * color2;
        UI_Label(ui, x, y, Str8Lit("Joining Game"), color, fade_in);
        y += VerticalAdvanceFontHeight(ui);
        
        if (menu->is_active)
        {
            if (HasFlag(game_state->flags, StateFlag_JoinedGame))
            {
                UI_OpenMenu(ui, GameMenuKind_EnterUsername);
            }
            else if (HasFlag(game_state->flags, StateFlag_FailedJoinGame))
            {
                UI_OpenMenu(ui, GameMenuKind_NetworkError);
            }
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_NetworkError)
    {
        f32 fade_in = menu->presence;
        
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        UI_Label(ui, x, y, Str8Lit("Lith Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.4f * game_state->render_context.screen.y;
        
        UI_Label(ui, x, y, Str8Lit("Encountered a Network Error"), Vec4(0.8f, 0.8f, 0.8f, 1.0f), fade_in);
        y += 2 * VerticalAdvanceFontHeight(ui);
        
        if (UI_Button(ui, x, y, Str8Lit("Return to Main Menu"), dt, fade_in))
        {
            UI_OpenMenu(ui, GameMenuKind_Main);
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_HostGame)
    {
        f32 fade_in = menu->presence;
        
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        UI_Label(ui, x, y, Str8Lit("Host Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.4f * game_state->render_context.screen.y;
        
        f32 color_change = Square(SinF(os->time.game_time));
        v4 color1 = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        v4 color2 = Vec4(0.8f, 0.8f, 0.8f, 1.0f);
        v4 color = color_change * color1 + (1 - color_change) * color2;
        UI_Label(ui, x, y, Str8Lit("Waiting Server Confirmation"), color, fade_in);
        y += VerticalAdvanceFontHeight(ui);
        
        if (menu->is_active)
        {
            if (HasFlag(game_state->flags, StateFlag_HostingGame))
            {
                UI_OpenMenu(ui, GameMenuKind_EnterUsername);
            }
            else if (HasFlag(game_state->flags, StateFlag_CantHost))
            {
                ClearFlag(game_state->flags, StateFlag_CantHost);
                UI_OpenMenu(ui, GameMenuKind_NetworkError);
            }
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_EnterUsername)
    {
        f32 fade_in = menu->presence;
        
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        UI_Label(ui, x, y, Str8Lit("Joining Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.4f * game_state->render_context.screen.y;
        
        UI_HostingGameMessage(game_state, ui, x, y, dt, fade_in);
        
        UI_Label(ui, x, y, Str8Lit("type a username:"), Vec4(0.8f, 0.8f, 0.8f, 1.0f), fade_in);
        y += VerticalAdvanceFontHeight(ui);
        
        ChangeActiveFont(ui, FontKind_InputField);
        UI_InputField(ui, InputFieldKind_PlayerUsername, x, y, dt, fade_in);
        
        y += 1.5f * VerticalAdvanceFontHeight(ui);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        
        if (UI_Button(ui, x, y, Str8Lit("Continue"), dt, fade_in))
        {
            Game_UI_InputField *input_field = ui->input_fields + InputFieldKind_PlayerUsername;
            if (input_field->size)
            {
                PushUsernameNetworkMessage(Str8(input_field->buffer, input_field->size));
                UI_OpenMenu(ui, GameMenuKind_UsernameConfirmation);
            }
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_UsernameConfirmation)
    {
        f32 fade_in = menu->presence;
        
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        UI_Label(ui, x, y, Str8Lit("Joining Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.4f * game_state->render_context.screen.y;
        
        UI_HostingGameMessage(game_state, ui, x, y, dt, fade_in);
        
        f32 color_change = Square(SinF(os->time.game_time));
        v4 color1 = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        v4 color2 = Vec4(0.8f, 0.8f, 0.8f, 1.0f);
        v4 color = color_change * color1 + (1 - color_change) * color2;
        UI_Label(ui, x, y, Str8Lit("Waiting Username Confirmation"), color, fade_in);
        y += VerticalAdvanceFontHeight(ui);
        
        
        if (menu->is_active)
        {
            if (HasFlag(game_state->flags, StateFlag_UsernameValid))
            {
                UI_OpenMenu(ui, GameMenuKind_JoinedPlayers);
            }
            else if (HasFlag(game_state->flags, StateFlag_UsernameInvalid))
            {
                ClearFlag(game_state->flags, StateFlag_UsernameInvalid);
                UI_OpenMenu(ui, GameMenuKind_InvalidUsername);
            }
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_InvalidUsername)
    {
        f32 fade_in = menu->presence;
        
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        UI_Label(ui, x, y, Str8Lit("Joining Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.4f * game_state->render_context.screen.y;
        
        UI_HostingGameMessage(game_state, ui, x, y, dt, fade_in);
        
        f32 color_change = Square(SinF(os->time.game_time));
        v4 color1 = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        v4 color2 = Vec4(0.8f, 0.8f, 0.8f, 1.0f);
        v4 color = color_change * color1 + (1 - color_change) * color2;
        UI_Label(ui, x, y, Str8Lit("Invalid username, try something else"), color, fade_in);
        y += VerticalAdvanceFontHeight(ui);
        
        if (UI_Button(ui, x, y, Str8Lit("Return"), dt, fade_in))
        {
            UI_OpenMenu(ui, GameMenuKind_EnterUsername);
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_JoinedPlayers)
    {
        f32 fade_in = menu->presence;
        
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        UI_Label(ui, x, y, Str8Lit("Joining Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        
        UI_HostingGameMessage(game_state, ui, x, y, dt, fade_in);
        
        // NOTE(fakhri): render the connected players
        
        f32 x_displacement = 0.3f;
        f32 y_displacement = 0.4f;
        local_persist v2 position_coefficients[] = {
            Vec2(x_displacement,         y_displacement), Vec2(1.0f - x_displacement,        y_displacement),
            Vec2(x_displacement,  1.0f - y_displacement), Vec2(1.0f - x_displacement, 1.0f - y_displacement),
        };
        
        for (u32 index = 0;
             index < ArrayCount(game_state->players);
             ++index)
        {
            Player *player = game_state->players + index;
            x = position_coefficients[index].x * game_state->render_context.screen.width;
            y = position_coefficients[index].y * game_state->render_context.screen.height;
            y += 0.2f * game_state->render_context.screen.height;
            if(player->joined)
            {
                UI_Label(ui, x, y, player->username, Vec4(0.95f, 0.95f, 0.95f, 1.0f), fade_in);
            }
            else
            {
                f32 color_change = Square(SinF(2 * os->time.game_time));
                v4 color1 = Vec4(0.6f, 0.6f, 0.6f, 1.0f);
                v4 color2 = Vec4(0.4f, 0.4f, 0.4f, 1.0f);
                v4 color = color_change * color1 + (1 - color_change) * color2;
                UI_Label(ui, x, y, Str8Lit("waiting player"), color, fade_in);
            }
        }
        
        if (menu->is_active)
        {
            if (game_state->players_joined_so_far == MAX_PLAYER_COUNT)
            {
                // NOTE(fakhri): enough players have joined
                StartGame(game_state);
                UI_CloseMenu(ui);
            }
        }
    }
    UI_MenuSectionEnd();
    
    //~
    UI_MenuSectionBegin(ui, GameMenuKind_HostInGameError)
    {
        Render_PushQuad(&game_state->render_context, Vec3(0,0, Meter(2)), Vec2(Meter(2), Meter(2)),Vec4(0.05f, 0.05f, 0.05f, 0.7f), CoordinateType_World);
        f32 fade_in = menu->presence;
        
        f32 x = 0.5f * game_state->render_context.screen.width;
        f32 y = 0.2f * game_state->render_context.screen.height;
        ChangeActiveCoordinates(ui, CoordinateType_Screen);
        ChangeActiveFont(ui, FontKind_MenuTitle);
        UI_Label(ui, x, y, Str8Lit("Lith Game"), Vec4(1.0f, 1.0f, 1.0f, 1.0f), fade_in);
        
        ChangeActiveFont(ui, FontKind_MenuItem);
        y = 0.4f * game_state->render_context.screen.y;
        
        UI_Label(ui, x, y, Str8Lit("Encountered a Network Error"), Vec4(0.8f, 0.8f, 0.8f, 1.0f), fade_in);
        y += 2 * VerticalAdvanceFontHeight(ui);
        
        if (HasFlag(game_state->flags, StateFlag_PlayerDisconnected))
        {
            UI_Label(ui, x, y, Str8Lit("A Player Disconnected"), Vec4(0.8f, 0.8f, 0.8f, 1.0f), fade_in);
            
            y += 2 * VerticalAdvanceFontHeight(ui);
        }
        
        if (UI_Button(ui, x, y, Str8Lit("Return to Main Menu"), dt, fade_in))
        {
            SetFlag(game_state->flags, StateFlag_EndGame);
        }
        
    }
    UI_MenuSectionEnd();
    
    UI_End(ui);
}