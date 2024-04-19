#include "opengl.h"

static VOID GlDebugOutput(INT32 source, INT32 type, UINT32 id, INT32 severity, INT32 length, PCSTR message,
                          CONST PVOID userParam)
{
#define X(out, type, name)                                                                                             \
    case GL_DEBUG_##type##_##name:                                                                                     \
        (out) = #name;                                                                                                 \
        break;

    PCSTR sourceText;
    switch (source)
    {
        X(sourceText, SOURCE, API);
        X(sourceText, SOURCE, WINDOW_SYSTEM);
        X(sourceText, SOURCE, SHADER_COMPILER);
        X(sourceText, SOURCE, THIRD_PARTY);
        X(sourceText, SOURCE, APPLICATION);
    default:
        X(sourceText, SOURCE, OTHER);
    }

    PCSTR typeText;
    switch (source)
    {
        X(typeText, TYPE, ERROR);
        X(typeText, TYPE, DEPRECATED_BEHAVIOR);
        X(typeText, TYPE, UNDEFINED_BEHAVIOR);
        X(typeText, TYPE, PORTABILITY);
        X(typeText, TYPE, PERFORMANCE);
        X(typeText, TYPE, MARKER);
        X(typeText, TYPE, PUSH_GROUP);
        X(typeText, TYPE, POP_GROUP);
    default:
        X(typeText, TYPE, OTHER);
    }

    LOG_LEVEL severityLevel;
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        severityLevel = LogLevelError;
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severityLevel = LogLevelWarning;
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severityLevel = LogLevelInfo;
        break;
    default:
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severityLevel = LogLevelDebug;
        break;
    }

    LogMessage(severityLevel, "OpenGL", (UINT64)PlatGetReturnAddress(), TRUE, "%s message from %s: %s", typeText,
               sourceText, message);

#undef X
}

VOID GlSetDebugCallback(VOID)
{
#ifdef PURPL_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(GlDebugOutput, NULL);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
}
