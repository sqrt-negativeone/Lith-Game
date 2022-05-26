
// NOTE(fakhri): IMPORTANT: we assume that all the platforms we target are LITTLE ENDIAN


internal void
AddCardToResidency(CardResidencyKind kind, Card_Type card_type)
{
    CardResidency *card_residency = host_context.residencies + kind;
    Assert(card_residency->count < ArrayCount(card_residency->cards));
    card_residency->cards[card_residency->count] = card_type;
    ++card_residency->count;
};

internal inline void
RemoveCardFromHand(Compact_Card_Hand *card_hand, u32 card_index)
{
    --card_hand->cards_count;
    card_hand->cards[card_index] = card_hand->cards[card_hand->cards_count];
}

internal void
BurnExtraCards(CardResidencyKind residency_kind)
{
    CardResidency *residency = host_context.residencies + residency_kind;
    CardResidency *burnt = host_context.residencies + CardResidencyKind_Burnt;
    
    // NOTE(fakhri): count the frequency of each card
    u32 card_number_freq[Card_Number_Count] = {};
    for(u32 card_index = 0;
        card_index < residency->count;
        ++card_index)
    {
        ++card_number_freq[residency->cards[card_index].number];
    }
    
    b32 should_burn = false;
    // NOTE(fakhri): mark the cards to be removed
    for(u32 card_index = 0;
        card_index < residency->count;
        ++card_index)
    {
        Card_Number card_number = residency->cards[card_index].number;
        if(card_number_freq[card_number] == Category_Count)
        {
            residency->cards[card_index].number = Card_Number_Count;
            AddCardToResidency(CardResidencyKind_Burnt, residency->cards[card_index]);
            should_burn = true;
        }
    }
    
    if(should_burn)
    {
        // NOTE(fakhri): remove the marked cards from the compact hand
        u32 min_empty = 0;
        for(u32 card_index = 0;
            card_index < residency->count;
            ++card_index)
        {
            Card_Number card_number = (u32)residency->cards[card_index].number;
            if(card_number != Card_Number_Count)
            {
                residency->cards[min_empty++] =  residency->cards[card_index];
            }
        }
        residency->count = min_empty;
    }
    
}

#if 0
internal void
AddTableCardsToPlayerHand(Compact_Card_Hand *punished_hand, Compact_Card_Hand *table,  Compact_Card_Hand *burnt_hand)
{
    // NOTE(fakhri): move the cards from the table to the hand
    for (u32 card_index = 0;
         card_index < table->cards_count;
         ++card_index)
    {
        Compact_Card_Type card = table->cards[card_index];
        AddCardToHand(punished_hand, card);
    }
    table->cards_count = 0;
    
    BurnExtraCards(punished_hand, burnt_hand);
}
#endif

internal void
GetPlayerUsernameWork(void *data)
{
    u64 tmp_player_index = (u64)data;
    Assert(tmp_player_index < ArrayCount(host_context.temporary_storage.players));
    Connected_Player *player = host_context.temporary_storage.players + tmp_player_index;
    Players_Storage *players_storage = &host_context.players_storage;
    
    // NOTE(fakhri): entrying username operation completed, check if the suername is unique
    for(;;)
    {
        if (ReceiveBuffer(player->socket, player->username, sizeof(player->username)))
        {
            b32 is_username_unique = true;
            for (u32 test_player_index = 0;
                 test_player_index < players_storage->count;
                 ++test_player_index)
            {
                Connected_Player *test_player = players_storage->players + test_player_index;
                if (strcmp (player->username, test_player->username) == 0)
                {
                    is_username_unique = false;
                    break;
                }
            }
            
            if (is_username_unique)
            {
                // NOTE(fakhri): good, check if we still have place for him
                W32_WaitForMutex(players_storage->players_mutex);
                if (players_storage->count < ArrayCount(players_storage->players))
                {
                    // NOTE(fakhri): add the player to the connected players array and notify the players
                    // about it
                    u32 player_id = players_storage->count;
                    players_storage->players[player_id] = *player;
                    BroadcastNewPlayerJoinedMessage(player, player_id);
                    ++players_storage->count;
                    SendConnectedPlayersList(player->socket);
                }
                else
                {
                    // NOTE(fakhri): sorry no room for you budd
                    MessageType type = HostMessage_HostFull;
                    SendBuffer(player->socket, &type, sizeof(type));
                    CloseSocket(player->socket);
                }
                W32_ReleaseMutex(players_storage->players_mutex);
                break;
            }
            else
            {
                MessageType type = HostMessage_InvalidUsername;
                SendBuffer(player->socket, &type, sizeof(type));
                if (!SendBuffer(player->socket, &type, sizeof(type)))
                {
                    CloseSocket(player->socket);
                    break;
                }
            }
        }
    }
    
    // NOTE(fakhri): try to increment the compeleted usernames count
    u64 old_count = host_context.completed_username_work;
    volatile u64 *destination = (volatile u64*)&host_context.completed_username_work;
    while(InterlockedCompareExchange(destination, old_count + 1, old_count) != old_count);
}

// NOTE(fakhri): main function for the thread that will serve as the host,
//it will be only active in case we are hosting the game
DWORD WINAPI HostMain(LPVOID param)
{
    host_context.players_storage.players_mutex = W32_CreateMutex();
    host_context.host_running = true;
    
    for (u32 player_index = 0;
         player_index < ArrayCount(host_context.players_storage.players);
         ++player_index)
    {
        host_context.players_storage.players[player_index].socket = InvalidSocket;
    }
    
    for(;;)
    {
        // TODO(fakhri): wait for the main thread to tell us to start the server
        Socket_Handle host_socket = OpenListenSocket(HOST_PORT);
        if(host_socket == InvalidSocket)
        {
            LogError("Couldn't open server listen socket");
            break;
        }
        
        // TODO(fakhri): tell the lobby that we exist
        
        // NOTE(fakhri): wait for enough players to join
        while(host_context.players_storage.count < MAX_PLAYER_COUNT)
        {
            u64 players_needed = MAX_PLAYER_COUNT - host_context.players_storage.count;
            for(u64 tmp_player_index = 0;
                tmp_player_index < players_needed;
                ++tmp_player_index)
            {
                Connected_Player *player = host_context.temporary_storage.players + tmp_player_index;
                player->socket = InvalidSocket;
                while(player->socket == InvalidSocket)
                {
                    player->socket = Accept(host_socket, 0, 0);
                }
                W32_PushWorkQueueEntry(GetPlayerUsernameWork, (void*)tmp_player_index);
            }
            // NOTE(fakhri): make sure that all the players entered a valid username
            while(host_context.completed_username_work < players_needed)
            {
                // NOTE(fakhri): make this thread help the worker thread
                // instead of just waiting
                W32_ProcessOneWorkQueueEntry();
            }
        }
        
        // NOTE(fakhri): we have all the players needed connected
        // TODO(fakhri): maybe we keep an uncompressed version of the cards in memory
        // that we work with, and only compress them when we want to send them over network
        
        u32 curr_player_id = 0;
        PlayerMove prev_move = {};
        Assert(Category_Count < 16);
        Assert(Card_Number_Count < 16);
        
        for (u8 category = 0, index = 0;
             category < Category_Count;
             ++category)
        {
            for (u8 number = Card_Number_Ace;
                 number < Card_Number_Count;
                 ++number, ++index)
            {
                AddCardToResidency(CardResidencyKind_Deck, MakeCardType(category, number));
            }
        }
        
        Rand_Ctx rand_ctx = MakeLineraRandomGenerator((u32)time(0));
        CardResidency *deck_residency = host_context.residencies + CardResidencyKind_Deck;
        Game_Step step = GameStep_ShuffleDeck;
        b32 stop = false;
        while(!stop)
        {
            switch(step)
            {
                case GameStep_ShuffleDeck:
                {
                    // NOTE(fakhri): shuffle the deck
                    for (u32 card_index = 0;
                         card_index < deck_residency->count - 1;
                         ++card_index)
                    {
                        u32 swap_index = NextRandomNumberMinMax(&rand_ctx, card_index, deck_residency->count);
                        Swap(Card_Type, deck_residency->cards[card_index], deck_residency->cards[swap_index]);
                    }
                    step = GameStep_SendDeckToPlayers;
                } break;
                case GameStep_SendDeckToPlayers:
                {
                    // NOTE(fakhri): send the deck to the players
                    // each player knows the what portion of the deck he gets based on his id
                    BroadcastShuffledDeckMessage();
                    
                    // NOTE(fakhri): split the deck between players
                    for(u32 player_index = 0, card_index = 0;
                        player_index < MAX_PLAYER_COUNT;
                        ++player_index)
                    {
                        CardResidencyKind player_residency = CardResidencyKind_Player0 + player_index;
                        for(u32 card_offset = 0;
                            card_offset < CARDS_PER_PLAYER;
                            ++card_offset, ++card_index)
                        {
                            AddCardToResidency(player_residency, deck_residency->cards[card_index]);
                        }
                        BurnExtraCards(player_residency);
                    }
                    
                    curr_player_id = NextRandomNumber(&rand_ctx);
                    step = GameStep_ChangePlayerTurn;
                } break;
                case GameStep_ChangePlayerTurn:
                {
                    // NOTE(fakhri): go in a cercle
                    ++curr_player_id; 
                    curr_player_id %= MAX_PLAYER_COUNT;
                    BroadcastChangeTurnMessage(curr_player_id);
                    step = GameStep_WaitForPlayerMove;
                } break;
                case GameStep_WaitForPlayerMove:
                {
                    Connected_Player *curr_player = host_context.players_storage.players + curr_player_id;
                    u32 prev_player_id = (curr_player_id - 1) % MAX_PLAYER_COUNT;
                    CardResidency *curr_player_residency = host_context.residencies + CardResidencyKind_Player0 + curr_player_id;
                    CardResidency *prev_player_residency = host_context.residencies + CardResidencyKind_Player0 + prev_player_id;
                    PlayerMove player_move;
                    if (ReceivePlayerMove(curr_player->socket, &player_move))
                    {
                        
                        Connected_Player *prev_player = host_context.players_storage.players + prev_player_id;
                        // NOTE(fakhri): update our state
                        switch(player_move.type)
                        {
                            case PlayerMove_PlayCard:
                            {
                                // TODO(fakhri): make sure that the player own the cards he says he played
                                // TODO(fakhri): remove the cards from his residency and add them to the table
                            } break;
                            case PlayerMove_QuestionCredibility:
                            {
                                // TODO(fakhri): implement this
                            } break;
                            default: NotImplemented; break;
                        }
                        prev_move = player_move;
                        // NOTE(fakhri): let all the players know what the move was
                        BroadcastPlayerMove(player_move);
                        // TODO(fakhri): check for winning condition!
                        step = GameStep_ChangePlayerTurn;
                    }
                    else
                    {
                        step = GameStep_InvalidMove;
                    }
                } break;
                case GameStep_Finished:
                {
                    
                } break;
                default:
                {
                    stop = true;
                    break;
                }
            }
        }
        
        // NOTE(fakhri): close the players that are still connected
        for (u32 player_index = 0;
             player_index < ArrayCount(host_context.players_storage.players);
             ++player_index)
        {
            Connected_Player *player = host_context.players_storage.players + player_index;
            if (player->socket != InvalidSocket)
            {
                CloseSocket(player->socket);
                player->socket = InvalidSocket;
            }
        }
        host_context.players_storage.count = 0;
        CloseSocket(host_socket);
    }
    
    return 0;
}
