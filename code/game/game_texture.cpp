#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

read_only global String8 textures_path[TextureID_Count] = {
    /*[TextureID_None]              */ Str8LitComp(""),
    /*[TextureID_Clover]            */ Str8LitComp("data/images/clovers.png"),
    /*[TextureID_Heart]             */ Str8LitComp("data/images/hearts.png"),
    /*[TextureID_Pikes]             */ Str8LitComp("data/images/pikes.png"),
    /*[TextureID_Tiles]             */ Str8LitComp("data/images/tiles.png"),
    /*[TextureID_BlackNumbers_Ace]  */ Str8LitComp("data/images/A_black.png"),
    /*[TextureID_BlackNumbers_2]    */ Str8LitComp("data/images/2_black.png"),
    /*[TextureID_BlackNumbers_3]    */ Str8LitComp("data/images/3_black.png"),
    /*[TextureID_BlackNumbers_4]    */ Str8LitComp("data/images/4_black.png"),
    /*[TextureID_BlackNumbers_5]    */ Str8LitComp("data/images/5_black.png"),
    /*[TextureID_BlackNumbers_6]    */ Str8LitComp("data/images/6_black.png"),
    /*[TextureID_BlackNumbers_7]    */ Str8LitComp("data/images/7_black.png"),
    /*[TextureID_BlackNumbers_8]    */ Str8LitComp("data/images/8_black.png"),
    /*[TextureID_BlackNumbers_9]    */ Str8LitComp("data/images/9_black.png"),
    /*[TextureID_BlackNumbers_10]   */ Str8LitComp("data/images/10_black.png"),
    /*[TextureID_BlackNumbers_Jack] */ Str8LitComp("data/images/J_black.png"),
    /*[TextureID_BlackNumbers_Queen]*/ Str8LitComp("data/images/Q_black.png"),
    /*[TextureID_BlackNumbers_King] */ Str8LitComp("data/images/K_black.png"),
    /*[TextureID_RedNumbers_Ace]    */ Str8LitComp("data/images/A_red.png"),
    /*[TextureID_RedNumbers_2]      */ Str8LitComp("data/images/2_red.png"),
    /*[TextureID_RedNumbers_3]      */ Str8LitComp("data/images/3_red.png"),
    /*[TextureID_RedNumbers_4]      */ Str8LitComp("data/images/4_red.png"),
    /*[TextureID_RedNumbers_5]      */ Str8LitComp("data/images/5_red.png"),
    /*[TextureID_RedNumbers_6]      */ Str8LitComp("data/images/6_red.png"),
    /*[TextureID_RedNumbers_7]      */ Str8LitComp("data/images/7_red.png"),
    /*[TextureID_RedNumbers_8]      */ Str8LitComp("data/images/8_red.png"),
    /*[TextureID_RedNumbers_9]      */ Str8LitComp("data/images/9_red.png"),
    /*[TextureID_RedNumbers_10]     */ Str8LitComp("data/images/10_red.png"),
    /*[TextureID_RedNumbers_Jack]   */ Str8LitComp("data/images/J_red.png"),
    /*[TextureID_RedNumbers_Queen]  */ Str8LitComp("data/images/Q_red.png"),
    /*[TextureID_RedNumbers_King]   */ Str8LitComp("data/images/K_red.png"),
    
    /*[TextureID_Jack + Category_Hearts]  */ Str8LitComp("data/images/jack_hearts.png"),
    /*[TextureID_Jack + Category_Tiles]   */ Str8LitComp("data/images/jack_tiles.png"),
    /*[TextureID_Jack + Category_Clovers] */ Str8LitComp("data/images/jack_clovers.png"),
    /*[TextureID_Jack + Category_Pikes]   */ Str8LitComp("data/images/jack_pikes.png"),
    
    /*[TextureID_Queen + Category_Hearts]  */ Str8LitComp("data/images/queen_hearts.png"),
    /*[TextureID_Queen + Category_Tiles]   */ Str8LitComp("data/images/queen_tiles.png"),
    /*[TextureID_Queen + Category_Clovers] */ Str8LitComp("data/images/queen_clovers.png"),
    /*[TextureID_Queen + Category_Pikes]   */ Str8LitComp("data/images/queen_pikes.png"),
    
    /*[TextureID_King + Category_Hearts]  */ Str8LitComp("data/images/king_hearts.png"),
    /*[TextureID_King + Category_Tiles]   */ Str8LitComp("data/images/king_tiles.png"),
    /*[TextureID_King + Category_Clovers] */ Str8LitComp("data/images/king_clovers.png"),
    /*[TextureID_King + Category_Pikes]   */ Str8LitComp("data/images/king_pikes.png"),
    
    /*[TextureID_Arrow]             */ Str8LitComp("data/images/arrow.png"),
    /*[TextureID_CardBack]          */ Str8LitComp("data/images/card_frame.png"),
    /*[TextureID_CardFrame]         */ Str8LitComp("data/images/card_back.png"),
};

internal TextureFormat2D
TextureFormatFromInternalFormat(GLuint format)
{
    TextureFormat2D result = TextureFormat2D_Null;
    switch(format)
    {
        default: break;
        case GL_RED:  result = TextureFormat2D_R8; break;
        case GL_RGB:  result = TextureFormat2D_RGB8; break;
        case GL_RGBA: result = TextureFormat2D_RGBA8; break;
    }
    return result;
}

internal GLint
GenericFormatFromTextureFormat2D(TextureFormat2D format)
{
    GLint result = 0;
    switch(format)
    {
        default: break;
        case TextureFormat2D_R8:    {result = GL_RED;}break;
        case TextureFormat2D_RGB8:  {result = GL_RGB;}break;
        case TextureFormat2D_RGBA8: {result = GL_RGBA;}break;
    }
    return result;
}

internal GLenum
BaseTypeFromTextureFormat2D(TextureFormat2D format)
{
    GLenum result = GL_UNSIGNED_BYTE;
    switch(format)
    {
        default: break;
        case TextureFormat2D_R8:    {result = GL_UNSIGNED_BYTE;}break;
        case TextureFormat2D_RGB8:  {result = GL_UNSIGNED_BYTE;}break;
        case TextureFormat2D_RGBA8: {result = GL_UNSIGNED_BYTE;}break;
    }
    return result;
}

internal GLint
InternalFormatFromTextureFormat2D(TextureFormat2D format)
{
    GLint result = 0;
    switch(format)
    {
        default:
        case TextureFormat2D_R8:    {result = GL_R8;}break;
        case TextureFormat2D_RGB8:  {result = GL_RGB8;}break;
        case TextureFormat2D_RGBA8: {result = GL_RGBA8;}break;
    }
    return result;
}

internal Texture2D
ReserveTexture2D(v2i32 size, TextureFormat2D format)
{
    Texture2D texture = {0};
    GLenum internal_fmt = InternalFormatFromTextureFormat2D(format);
    glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
    glTextureStorage2D(texture.id, 1, internal_fmt, size.x, size.y);
    
    switch(format)
    {
        default: break;
        case TextureFormat2D_R8:
        {
            GLint swizzle_mask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
            glTextureParameteriv(texture.id, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);
        }break;
    }
    
    texture.format = format;
    texture.size = size;
    return texture;
}

internal void
FillTexture2D(Texture2D texture, v2i32 position, v2i32 size, String8 data)
{
    glTextureSubImage2D(texture.id, 0, position.x, position.y, size.x, size.y, GenericFormatFromTextureFormat2D(texture.format),
                        BaseTypeFromTextureFormat2D(texture.format),
                        data.str);
    glGenerateTextureMipmap(texture.id);
}

internal void
ReleaseTexture2D(Texture2D texture)
{
    glDeleteTextures(1, &texture.id);
}

internal void
LoadTexture(Render_Context *render_context, TextureID texture_id)
{
    Assert(texture_id < TextureID_Count);
    if (texture_id == TextureID_None) return;
    String8 image_path = textures_path[texture_id];
    Texture2D *texture = render_context->textures + texture_id;
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    i32 nb_channels;
    void* data = stbi_load(image_path.cstr, &texture->size.width, &texture->size.height, &nb_channels, 0);
    if (data)
    {
        GLenum format = 0;
        switch(nb_channels)
        {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default: LogWarning("channels count %d not handled when loading image %s", nb_channels, image_path.str);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, format, texture->size.width, texture->size.height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        
        texture->format = TextureFormatFromInternalFormat(format);
    }
    else
    {
        LogWarning("couldn't load image %s", image_path.str);
    }
}
