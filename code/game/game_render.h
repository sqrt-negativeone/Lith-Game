/* date = January 29th 2022 11:29 am */

#ifndef GAME_RENDER_H
#define GAME_RENDER_H

struct Frensh_Suited_Cards_Texture
{
    Texture2D card_frame_texture;
    Texture2D card_back_texture;
    
    Texture2D clovers_up;
    Texture2D hearts_up;
    Texture2D pikes_up;
    Texture2D clovers_down;
    Texture2D hearts_down;
    Texture2D pikes_down;
    Texture2D tiles;
    
    Texture2D black_numbers_up[13];
    Texture2D black_numbers_down[13];
    Texture2D red_numbers_up[13];
    Texture2D red_numbers_down[13];
    
    Texture2D jacks[4];
    Texture2D queens[4];
    Texture2D kings[4];
};

#endif //GAME_RENDER_H
