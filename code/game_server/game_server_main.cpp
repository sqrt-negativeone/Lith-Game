
// NOTE(fakhri): IMPORTANT: we assume that all the platforms we target are LITTLE ENDIAN

internal inline void
AddCardToHand(Compact_Card_Hand *card_hand, Compact_Card_Type card)
{
    Assert(card_hand->cards_count < ArrayCount(card_hand->cards));
    u32 card_index = card_hand->cards_count++;
    card_hand->cards[card_index] = card;
}

internal inline void
RemoveCardFromHand(Compact_Card_Hand *card_hand, u32 card_index)
{
    --card_hand->cards_count;
    card_hand->cards[card_index] = card_hand->cards[card_hand->cards_count];
}

internal inline void
SwapCompactCards(Compact_Card_Type *a, Compact_Card_Type *b)
{
    Compact_Card_Type tmp = *a;
    *a = *b;
    *b = tmp;
}

internal void
BurnExtraCards(Compact_Card_Hand *compact_hand, Compact_Card_Hand *burnt_hand)
{
    // NOTE(fakhri): unpack the card hand
    Card_Hand card_hand;
    for (u32 card_index = 0;
         card_index < compact_hand->cards_count;
         ++card_index)
    {
        card_hand.cards[card_index] = UnpackCompactCardType(compact_hand->cards[card_index]);
    }
    card_hand.cards_count = compact_hand->cards_count;
    
    // NOTE(fakhri): count the frequency of each card
    u32 card_number_freq[Card_Number_Count] = {};
    for(u32 card_index = 0;
        card_index < card_hand.cards_count;
        ++card_index)
    {
        u32 card_number = (u32)card_hand.cards[card_index].number;
        ++card_number_freq[card_number];
    }
    
    b32 should_burn = false;
    // NOTE(fakhri): mark the cards to be removed
    for(u32 card_index = 0;
        card_index < card_hand.cards_count;
        ++card_index)
    {
        u32 card_number = (u32)card_hand.cards[card_index].number;
        if(card_number_freq[card_number] == Category_Count)
        {
            card_hand.cards[card_index].number = Card_Number_Count;
            AddCardToHand(burnt_hand, compact_hand->cards[card_index]);
            should_burn = true;
        }
    }
    
    if(should_burn)
    {
        // NOTE(fakhri): remove the marked cards from the compact hand
        u32 min_empty = 0;
        for(u32 card_index = 0;
            card_index < card_hand.cards_count;
            ++card_index)
        {
            
            u32 card_number = (u32)card_hand.cards[card_index].number;
            if(card_number != Card_Number_Count)
            {
                SwapCompactCards(compact_hand->cards + card_index, compact_hand->cards + min_empty);
                ++min_empty;
            }
        }
    }
    
}

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

DWORD WINAPI PlayerUsernameThread(LPVOID param)
{
    Assert(param);
    
    Connected_Player *player = (Connected_Player *)param;
    Players_Storage *players_storage = &host_context.players_storage;
    
    // NOTE(fakhri): entrying username operation completed, check if the suername is unique
    while(host_context.host_running)
    {
        if (!ReceiveBuffer(player->socket, player->username, sizeof(player->username)))
        {
            break;
        }
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
            WaitForSingleObject(players_storage->players_mutex, INFINITE);
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
                MessageType type = MessageType_From_Host_Host_Full;
                SendBuffer(player->socket, &type, sizeof(type));
                closesocket(player->socket);
            }
            ReleaseMutex(players_storage->players_mutex);
            ReleaseSemaphore(host_context.usernames_done_sempahore, 1, 0);
            break;
        }
        else
        {
            MessageType type = MessageType_From_Host_Invalid_Username;
            SendBuffer(player->socket, &type, sizeof(type));
            if (!SendBuffer(player->socket, &type, sizeof(type)))
            {
                closesocket(player->socket);
            }
        }
    }
    
    return 0;
}

// NOTE(fakhri): main function for the thread that will serve as the host,
//it will be only active in case we are hosting the game
DWORD WINAPI HostMain(LPVOID param)
{
    host_context.players_storage.players_mutex = CreateMutexA(0, FALSE, 0);
    host_context.host_running = true;
    
    
    for (u32 player_index = 0;
         player_index < ArrayCount(host_context.players_storage.players);
         ++player_index)
    {
        host_context.players_storage.players[player_index].socket = INVALID_SOCKET;
    }
    
    for(;;)
    {
        
        // NOTE(fakhri): wait for the main thread to tell us to start the server
#if 0
        {
            // TODO(fakhri): see if there is a better way to do it?
            b32 undesired_value = 0;
            b32 captured_value = host_context.host_running;
            while(undesired_value == captured_value)
            {
                WaitOnAddress(&host_context.host_running, &undesired_value, sizeof(host_context.host_running), INFINITE);
                captured_value = host_context.host_running;
            }
        }
#endif
        
        SOCKET host_socket = OpenListenSocket(HOST_PORT);
        if(host_socket == INVALID_SOCKET)
        {
            break;
        }
        
        // TODO(fakhri): tell the lobby that we exist
        
        host_context.usernames_done_sempahore = CreateSemaphoreA(0, MAX_PLAYER_COUNT, MAX_PLAYER_COUNT, 0);
        // NOTE(fakhri): wait for enough players to join
        while(host_context.players_storage.count < MAX_PLAYER_COUNT)
        {
            
            u32 players_needed = MAX_PLAYER_COUNT - host_context.players_storage.count;
            for(u32 tmp_player_index = 0;
                tmp_player_index < players_needed;
                ++tmp_player_index)
            {
                
                Connected_Player *player = host_context.temporary_storage.players + tmp_player_index;
                player->socket = INVALID_SOCKET;
                while(player->socket == INVALID_SOCKET && host_context.host_running)
                {
                    player->socket = accept(host_socket, 0, 0);
                }
                
                WaitForSingleObject(host_context.usernames_done_sempahore, INFINITE);
                // NOTE(fakhri): create a thread that will get the username from the player
                HANDLE username_thread_handle = CreateThread(0, 0, PlayerUsernameThread, player, 0, 0);
                CloseHandle(username_thread_handle);
                
            }
            
            // NOTE(fakhri): make sure that all the players entered a valid username
            for (u32 index = 0;
                 index < players_needed;
                 ++index)
            {
                WaitForSingleObject(host_context.usernames_done_sempahore, INFINITE);
            }
        }
        
        u32 curr_player_id = 0;
        Player_Move prev_move = {};
        Compact_Card_Hand burnt_cards = {};
        Compact_Card_Hand table = {};
        Compact_Card_Type deck[DECK_CARDS_COUNT];
        Assert(Category_Count < 16);
        Assert(Card_Number_Count < 16);
        for (u8 category_index = Category_Hearts, card_index = 0;
             category_index < Category_Count;
             ++category_index)
        {
            for (u8 number_index = Card_Number_Ace;
                 number_index < Card_Number_Count;
                 ++number_index)
            {
                deck[card_index++] = MakeCompactCardType((Card_Category)category_index, (Card_Number)number_index);
            }
        }
        
        srand((u32)time(0));
        Game_Step step = Game_Step_Shuffle_Deck;
        
        b32 game_still_running = true;
        while(host_context.host_running && game_still_running)
        {
            switch(step)
            {
                case Game_Step_Shuffle_Deck:
                {
                    // NOTE(fakhri): shuffle the deck
                    for (u32 card_index = 0;
                         card_index < ArrayCount(deck) - 1;
                         ++card_index)
                    {
                        u32 random_number = rand();
                        u32 swap_index = card_index + random_number / (RAND_MAX / (ArrayCount(deck) - card_index) + 1);
                        
                        SwapCompactCards(deck + card_index, deck + swap_index);
                    }
                    
                    step = Game_Step_Send_Deck_To_Players;
                } break;
                case Game_Step_Send_Deck_To_Players:
                {
                    // NOTE(fakhri): send the deck to the players
                    // each player knows the what portion of the deck he gets based on his id
                    BroadcastShuffledDeckMessage(deck, sizeof(deck));
                    
                    // NOTE(fakhri): split the deck between players
                    for(u32 player_index = 0, card_index = 0;
                        player_index < MAX_PLAYER_COUNT;
                        ++player_index)
                    {
                        Connected_Player *player = host_context.players_storage.players + player_index;
                        for(u32 card_offset = 0;
                            card_offset < CARDS_PER_PLAYER;
                            ++card_offset, ++card_index)
                        {
                            AddCardToHand(&player->hand, deck[card_index]);
                        }
                        BurnExtraCards(&player->hand, &burnt_cards);
                    }
                    
                    curr_player_id = rand();
                    step = Game_Step_Change_Player_Turn;
                } break;
                case Game_Step_Change_Player_Turn:
                {
                    // NOTE(fakhri): go in a cercle
                    ++curr_player_id; 
                    curr_player_id %= MAX_PLAYER_COUNT;
                    BroadcastChangeTurnMessage(curr_player_id);
                    step = Game_Step_Wait_For_Player_Move;
                } break;
                case Game_Step_Wait_For_Player_Move:
                {
                    Connected_Player *curr_player = host_context.players_storage.players + curr_player_id;
                    Player_Move player_move;
                    ReceivePlayerMove(curr_player->socket, &player_move);
                    
                    
                    u32 prev_player_id = (curr_player_id - 1) % MAX_PLAYER_COUNT;
                    Connected_Player *prev_player = host_context.players_storage.players + prev_player_id;
                    
                    // NOTE(fakhri): update our state
                    {
                        if (player_move.type == Player_Move_Question_Credibility)
                        {
                            if (prev_move.type == Player_Move_Play_Card)
                            {
                                if (prev_move.actual_card != prev_move.claimed_card)
                                {
                                    // NOTE(fakhri): you got him, the liar
                                    AddTableCardsToPlayerHand(&prev_player->hand, &table, &burnt_cards);
                                }
                                else
                                {
                                    // NOTE(fakhri): oups.. he was telling the truth this time
                                    AddTableCardsToPlayerHand(&curr_player->hand, &table, &burnt_cards);
                                }
                            }
                            else
                            {
                                Assert(!"HUH? WHAT HAPPENED HERE?");
                            }
                        }
                        else if (player_move.type == Player_Move_Play_Card)
                        {
                            u32 card_index;
                            for (card_index = 0;
                                 card_index < curr_player->hand.cards_count;
                                 ++card_index)
                            {
                                if (curr_player->hand.cards[card_index] == player_move.actual_card)
                                {
                                    break;
                                }
                            }
                            
                            Assert(card_index < curr_player->hand.cards_count);
                            if (card_index < curr_player->hand.cards_count)
                            {
                                AddCardToHand(&table, player_move.actual_card);
                                RemoveCardFromHand(&table, card_index);
                            }
                            else
                            {
                                Assert(!"the player trying to cheat?");
                            }
                        }
                        
                        prev_move = player_move;
                    }
                    // NOTE(fakhri): let all the players know what the move was
                    BroadcastPlayerMove(player_move);
                    if (prev_player->hand.cards_count == 0)
                    {
                        // NOTE(fakhri): prev player won!
                        // tell the players that he won
                        BroadcastPlayerWon(prev_player_id);
                        
                        // NOTE(fakhri): a host will only run one game at a time
                        // players should reconnect to the host to play again
                        game_still_running = false;
                    }
                    step = Game_Step_Change_Player_Turn;
                } break;
            }
        }
        CloseHandle(host_context.usernames_done_sempahore);
        
        // NOTE(fakhri): close the players that are still connected
        for (u32 player_index = 0;
             player_index < ArrayCount(host_context.players_storage.players);
             ++player_index)
        {
            Connected_Player *player = host_context.players_storage.players + player_index;
            if (player->socket != INVALID_SOCKET)
            {
                closesocket(player->socket);
                player->socket = INVALID_SOCKET;
            }
        }
        host_context.players_storage.count = 0;
        closesocket(host_socket);
    }
    
    return 0;
}
