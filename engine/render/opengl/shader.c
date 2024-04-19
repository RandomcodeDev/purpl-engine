#include "opengl.h"

static UINT32 LoadShader(UINT32 Type, PCSTR EntryPoint, PCSTR Name)
{
    CHAR Buffer[512] = {0};
    INT32 Success = 0;
    UINT32 Shader = GL_INVALID_VALUE;

    LogInfo("Loading shader %s with entry point %s", Name, EntryPoint);

    CHAR Suffix = 'v';
    switch (Type)
    {
    case GL_VERTEX_SHADER:
        Suffix = 'v';
        break;
    case GL_FRAGMENT_SHADER:
        Suffix = 'p';
        break;
    }

    UINT64 ShaderSourceSize = 0;
    // Extra is used to NUL-terminate
    PBYTE ShaderSource =
        FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "opengl/%s.%cs.glsl", Name, Suffix), 0, 0, &ShaderSourceSize, 1);
    if (!ShaderSource || !ShaderSourceSize)
    {
        LogError("Failed to load shader %s", Name);
        goto Done;
    }

    LogInfo("Compiling shader %s", Name);
    Shader = glCreateShader(Type);
    glShaderSource(Shader, 1, &(PCSTR)ShaderSource, NULL);
    glCompileShader(Shader);
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);
    if (!Success)
    {
        glGetShaderInfoLog(Shader, PURPL_ARRAYSIZE(Buffer), NULL, Buffer);
        glDeleteShader(Shader);
        LogError("Failed to load shader %s: %s", Name, Buffer);
        Shader = GL_INVALID_VALUE;
        goto Done;
    }

Done:
    if (ShaderSource)
    {
        CmnFree(ShaderSource);
    }
    return Shader;
}

UINT64 GlLoadShader(_In_ PCSTR Name)
{
    CHAR Buffer[512] = {0};

    LogInfo("Loading OpenGL shader %s", Name);

    UINT32 VertexShader = LoadShader(GL_VERTEX_SHADER, "VertexMain", Name);
    UINT32 PixelShader = LoadShader(GL_FRAGMENT_SHADER, "PixelMain", Name);

    UINT32 Program = glCreateProgram();
    if (Program == GL_INVALID_VALUE)
    {
        LogError("Failed to create shader program: %d", glGetError());
        return GL_INVALID_VALUE;
    }

    glAttachShader(Program, VertexShader);
    glAttachShader(Program, PixelShader);
    glLinkProgram(Program);

    INT32 Success;
    glGetProgramiv(Program, GL_LINK_STATUS, &Success);
    if (!Success)
    {
        glGetProgramInfoLog(Program, PURPL_ARRAYSIZE(Buffer), NULL, Buffer);
        LogError("Failed to link shader program %s: %s", Name, Buffer);
        return GL_INVALID_VALUE;
    }

    glDetachShader(Program, VertexShader);
    glDeleteShader(VertexShader);
    glDetachShader(Program, PixelShader);
    glDeleteShader(PixelShader);

    glObjectLabel(GL_PROGRAM, Program, (INT32)strlen(Name), Name);

    return Program;
}

VOID GlDestroyShader(_In_ UINT64 Shader)
{
    glDeleteProgram((UINT32)Shader);
}

VOID GlSetUniform(RENDER_HANDLE Shader, PCSTR Name, PVOID Value, UINT32 Type, UINT32 Count)
{
#define XX1(Location, Type, Suffix, Value)                                                                             \
    case GL_##Type:                                                                                                    \
        glUniform1##Suffix(Location, (Value)[0]);                                                                      \
        break;
#define XX2(Location, Type, Suffix, Values)                                                                            \
    case GL_##Type:                                                                                                    \
        glUniform2##Suffix(Location, (Values)[0], (Values)[1]);                                                        \
        break;
#define XX3(Location, Type, Suffix, Values)                                                                            \
    case GL_##Type:                                                                                                    \
        glUniform3##Suffix(Location, (Values)[0], (Values)[1], (Values)[2]);                                           \
        break;
#define XX4(Location, Type, Suffix, Values)                                                                            \
    case GL_##Type:                                                                                                    \
        glUniform4##Suffix(Location, (Values)[0], (Values)[1], (Values)[2], (Values)[3]);                              \
        break;
#define X(Location, Type, Count, Values)                                                                               \
    case (Count):                                                                                                      \
        switch ((Type))                                                                                                \
        {                                                                                                              \
            XX##Count(Location, INT, i, (PINT32)Values);                                                               \
            XX##Count(Location, FLOAT, f, (PFLOAT)Values);                                                             \
        }                                                                                                              \
        break;

    LogTrace("Setting uniform %s in shader %u with %u type %u value(s)", Name, (UINT32)Shader, Count, Type);

    glUseProgram((UINT32)Shader);

    INT32 uniformLocation = glGetUniformLocation((UINT32)Shader, Name);
    if (uniformLocation == GL_INVALID_VALUE)
    {
        LogError("Failed to set uniform %s", Name);
        return;
    }

    switch (Count)
    {
    default: // default is 1
        X(uniformLocation, Type, 1, Value);
        X(uniformLocation, Type, 2, Value);
        X(uniformLocation, Type, 3, Value);
        X(uniformLocation, Type, 4, Value);
    }

    glUseProgram(0);

#undef X
#undef XX1
#undef XX2
#undef XX3
#undef XX4
}

VOID GlSetMatrixUniform(RENDER_HANDLE Shader, PCSTR Name, mat4 Value)
{
    LogTrace("Setting mat4 uniform %s in shader %u", Name, (UINT32)Shader);

    glUseProgram((UINT32)Shader);

    INT32 uniformLocation = glGetUniformLocation((UINT32)Shader, Name);
    if (uniformLocation == GL_INVALID_VALUE)
    {
        LogError("Failed to set uniform %s", Name);
        return;
    }

    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (PFLOAT)Value);

    glUseProgram(0);
}
