
internal void
PushCreateConnectToServerMessage(String8 server_address)
{
    Message *message = os->BeginPlayerMessageQueueWrite();
    message->type = PlayerMessage_ConnectToHost;
    Assert(server_address.size < ArrayCount(message->buffer));
    MemoryCopy(message->buffer, server_address.str, server_address.size);
    message->server_address = Str8(message->buffer, server_address.size);
    os->EndPlayerMessageQueueWrite();
}

internal void
PushUsernameNetworkMessage(String8 username)
{
    Message *message = os->BeginPlayerMessageQueueWrite();
    Assert(username.size < ArrayCount(message->buffer));
    MemoryCopy(message->buffer, username.str, username.size);
    message->type     = PlayerMessage_Username;
    message->username = Str8(message->buffer, username.size);
    os->EndPlayerMessageQueueWrite();
}

internal void
PushPlayedCardNetworkMessage(Game_State *game_state)
{
    Message *message = os->BeginPlayerMessageQueueWrite();
    message->type = PlayerMessage_PlayCard;
    message->player_move.played_cards_count = game_state->prev_played_cards_count;
    message->player_move.actual_cards = (Compact_Card_Type *)message->buffer;
    
    // NOTE(fakhri): fill the actual played cards array
    {
        Residency *table_res = game_state->residencies + ResidencyKind_Table;
        for (u32 index = 0;
             index < message->player_move.played_cards_count;
             ++index)
        {
            EntityID entity_id = table_res->entity_ids[table_res->entity_count - 1 - index];
            Card_Type card_type = game_state->entities[entity_id].card_type;
            message->player_move.actual_cards[index] = MakeCompactCardType(card_type.category, card_type.number);
        }
    }
    
    message->player_move.declared_number = game_state->declared_number;
    os->EndPlayerMessageQueueWrite();
}

internal void
PushQuestionCredibilityNetworkMessage()
{
    Message *message = os->BeginPlayerMessageQueueWrite();
    message->type = PlayerMessage_QuestionCredibility;
    os->EndPlayerMessageQueueWrite();
}

#if 0
internal Message
CreateFetchAvailableHostsMessage(Hosts_Storage *hosts_storage)
{
    Message result = {};
    result.type = PlayerMessage_FetchHosts;
    result.hosts_storage = hosts_storage;
    return result;
}

#endif
