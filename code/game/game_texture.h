/* date = March 29th 2022 10:33 pm */

#ifndef GAME_TEXTURE_H
#define GAME_TEXTURE_H


enum TextureFormat2D
{
    TextureFormat2D_Null,
    TextureFormat2D_R8,
    TextureFormat2D_RGB8,
    TextureFormat2D_RGBA8,
    TextureFormat2D_COUNT
};

struct Texture2D
{
    U32 id;
    TextureFormat2D format;
    v2i32 size;
};

#endif //GAME_TEXTURE_H
