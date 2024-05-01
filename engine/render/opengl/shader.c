#include "opengl.h"

static UINT32 LoadShader(UINT32 Type, PCSTR EntryPoint, PCSTR Name)
{
    CHAR Buffer[512] = {0};
    INT32 Success = 0;
    UINT32 Shader = GL_INVALID_VALUE;

    LogInfo("Loading shader %s with entry point %s", Name, EntryPoint);

    CHAR Prefix = 'v';
    switch (Type)
    {
    case GL_VERTEX_SHADER:
        Prefix = 'v';
        break;
    case GL_FRAGMENT_SHADER:
        Prefix = 'p';
        break;
    }

    UINT64 ShaderSourceSize = 0;
    // Extra is used to NUL-terminate
    PBYTE ShaderSource =
        FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "opengl/%s.%cs.glsl", Name, Prefix), 0, 0, &ShaderSourceSize, 1);
    if (!ShaderSource || !ShaderSourceSize)
    {
        LogError("Failed to load shader %s", Name);
        goto Done;
    }

    LogInfo("Compiling shader %s", Name);
    Shader = glCreateShader(Type);
    glShaderSource(Shader, 1, (PCSTR *)&ShaderSource, NULL);
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

UINT32 GlCreateUniformBuffer(UINT32 Size)
{
    LogInfo("Creating uniform buffer");

    UINT32 Buffer = GL_INVALID_VALUE;
    glGenBuffers(1, &Buffer);
    if (Buffer == GL_INVALID_VALUE)
    {
        CmnError("Failed to create uniform buffer: %d", glGetError());
    }

    glBindBuffer(GL_UNIFORM_BUFFER, Buffer);
    glBufferData(GL_UNIFORM_BUFFER, Size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glObjectLabel(GL_BUFFER, Buffer, 14, "Uniform buffer");

    return Buffer;
}

VOID GlWriteUniformBuffer(UINT32 UniformBuffer, UINT32 Offset, PVOID Data, UINT32 Size)
{
    LogTrace("Writing %u bytes at offset 0x%X in uniform buffer %u", Size, Offset, UniformBuffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, UniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, Offset, Size, Data);
}
