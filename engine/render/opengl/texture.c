#include "opengl.h"

RENDER_HANDLE GlUseTexture(_In_ PTEXTURE Texture, _In_z_ PCSTR Name)
{
    UINT32 TextureHandle = 0;
    glGenTextures(1, &TextureHandle);

    glBindTexture(GL_TEXTURE_2D, TextureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Texture->Width, Texture->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture->Pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glObjectLabel(GL_TEXTURE_2D, TextureHandle, (UINT32)strlen(Name), Name);

    glBindTexture(GL_TEXTURE_2D, 0);

    return TextureHandle;
}

VOID GlReleaseTexture(_In_ RENDER_HANDLE Handle)
{
    UINT32 Texture = (UINT32)Handle;
    glDeleteTextures(1, &Texture);
}

