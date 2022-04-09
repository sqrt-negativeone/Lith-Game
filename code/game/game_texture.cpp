#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

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

internal Texture2D
LoadTexture(String8 image_path)
{
    Texture2D result = {};
    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    i32 nb_channels;
    void* data = stbi_load(image_path.cstr, &result.size.width, &result.size.height, &nb_channels, 0);
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
        glTexImage2D(GL_TEXTURE_2D, 0, format, result.size.width, result.size.height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        
        result.format = TextureFormatFromInternalFormat(format);
    }
    else
    {
        LogWarning("couldn't load image %s", image_path.str);
    }
    return result;
}
