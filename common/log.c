/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Modifications to naming and formatting by Randomcode Developers
 */

#include "log.h"
#include "platform/async.h"

#define LOG_MAX_CALLBACKS 32

typedef struct LOG_CALLBACK
{
    PFN_LOG_LOG Log;
    void *Data;
    int Level;
} LOG_CALLBACK;

static struct LOG_STATE
{
    void *Data;
    PFN_LOG_LOCK Lock;
    LOG_LEVEL Level;
    bool Quiet;
    LOG_CALLBACK Callbacks[LOG_MAX_CALLBACKS];
} LogState;

static CONST char *LevelStrings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

#ifdef LOG_USE_COLOR
static CONST char *LevelColours[] = {"\x1b[38;5;197m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"};
#endif

#ifndef PURPL_SWITCH
static void StdoutCallback(LOG_EVENT *Event)
{
    char Buffer[64] = {0};
    Buffer[strftime(Buffer, sizeof(Buffer), "%H:%M:%S", Event->Time)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(Event->Data, "%s \x1b[38;5;213m%s\x1b[0m %s%-5s\x1b[0m \x1b[90m%s:", Buffer, AsCurrentThread->Name,
            LevelColours[Event->Level], LevelStrings[Event->Level], Event->File);
    if (Event->HexLine)
        fprintf(Event->Data, "0x%llX:\x1b[0m ", (UINT64)Event->Line);
    else
        fprintf(Event->Data, "%lld:\x1b[0m ", (INT64)Event->Line);
#else
    fprintf(Event->Data, "%s %s %-5s %s:", Buffer, AsCurrentThread->Name, LevelStrings[Event->Level], Event->File);
    if (Event->HexLine)
        fprintf(Event->Data, "0x%llX: ", (UINT64)Event->Line);
    else
        fprintf(Event->Data, "%lld: ", (INT64)Event->Line);
#endif
    vfprintf(Event->Data, Event->Format, Event->ArgList);
    fprintf(Event->Data, "\n");
    fflush(Event->Data);
}
#endif

#ifdef PURPL_HAVE_PLATPRINT
static void PlatPrintCallback(LOG_EVENT *Event)
/*++

Routine Description:

    Sends log messages to the debug console using
    PlatPrint.

Arguments:

    Event - Log event information to use.

Return Value:

    None.

--*/
{
    char Time[64] = {0};
    char Message[1024] = {0};
    char All[1024] = {0};

    Time[strftime(Time, sizeof(Time), "%H:%M:%S", Event->Time)] = 0;

    vsnprintf(Message, sizeof(Message), Event->Format, Event->ArgList);
    if (Event->HexLine)
        snprintf(All, sizeof(All), "%s %s %-5s %s:0x%llX: %s\n", Time, AsCurrentThread->Name,
                 LogGetLevelString(Event->Level), Event->File, (UINT64)Event->Line, Message);
    else
        snprintf(All, sizeof(All), "%s %s %-5s %s:%lld: %s\n", Time, AsCurrentThread->Name,
                 LogGetLevelString(Event->Level), Event->File, (INT64)Event->Line, Message);

    PlatPrint(All);
}
#endif

static void FileCallback(LOG_EVENT *Event)
{
    char Buffer[64] = {0};
    Buffer[strftime(Buffer, sizeof(Buffer), "%Y-%m-%d %H:%M:%S", Event->Time)] = '\0';
    fprintf(Event->Data, "%s %s %-5s %s:", Buffer, AsCurrentThread->Name, LevelStrings[Event->Level], Event->File);
    if (Event->HexLine)
        fprintf(Event->Data, "0x%llX: ", (UINT64)Event->Line);
    else
        fprintf(Event->Data, "%lld: ", (INT64)Event->Line);
    vfprintf(Event->Data, Event->Format, Event->ArgList);
    fprintf(Event->Data, "\n");
    fflush(Event->Data);
}

static void LogLock(void)
{
    if (LogState.Lock)
    {
        LogState.Lock(true, LogState.Data);
    }
}

static void LogUnlock(void)
{
    if (LogState.Lock)
    {
        LogState.Lock(false, LogState.Data);
    }
}

CONST
char *LogGetLevelString(LOG_LEVEL Level)
{
    return LevelStrings[Level];
}

void LogSetLock(PFN_LOG_LOCK Lock, void *Data)
{
    LogState.Lock = Lock;
    LogState.Data = Data;
}

void LogSetLevel(LOG_LEVEL Level)
{
    LogState.Level = Level;
}

LOG_LEVEL
LogGetLevel(VOID)
{
    return LogState.Level;
}

void LogSetQuiet(bool Quiet)
{
    LogState.Quiet = Quiet;
}

int LogAddCallback(PFN_LOG_LOG Callback, void *Data, LOG_LEVEL Level)
{
    for (int i = 0; i < LOG_MAX_CALLBACKS; i++)
    {
        if (!LogState.Callbacks[i].Log)
        {
            LogState.Callbacks[i] = (LOG_CALLBACK){Callback, Data, Level};
            return 0;
        }
    }
    return -1;
}

int LogAddFile(FILE *File, LOG_LEVEL Level)
{
    return LogAddCallback(FileCallback, File, Level);
}

static void InitEvent(LOG_EVENT *Event, void *Data)
{
    if (!Event->Time)
    {
        time_t t = time(NULL);
        Event->Time = localtime(&t);
    }
    Event->Data = Data;
}

void LogMessage(LOG_LEVEL Level, CONST char *File, uint64_t Line, bool HexLine, CONST char *Format, ...)
{
    LOG_EVENT Event = {
        .Format = Format,
        .File = File,
        .Line = Line,
        .HexLine = HexLine,
        .Level = Level,
    };

    if (Event.File)
    {
        while (strstr(Event.File, "purpl-engine"))
        {
            Event.File = strstr(Event.File, "purpl-engine") + 12 + 1; // also skip the slash
            if (Event.File > File + strlen(File))
            {
                Event.File = File;
            }
        }
    }

    LogLock();

    if (!LogState.Quiet && Level >= LogState.Level)
    {
        InitEvent(&Event, stderr);
        va_start(Event.ArgList, Format);

#ifndef PURPL_SWITCH
        StdoutCallback(&Event);
#endif

#ifdef PURPL_HAVE_PLATPRINT
        PlatPrintCallback(&Event);
#endif

        va_end(Event.ArgList);
    }

    for (int i = 0; i < LOG_MAX_CALLBACKS && LogState.Callbacks[i].Log; i++)
    {
        LOG_CALLBACK *cb = &LogState.Callbacks[i];
        if (Level >= cb->Level)
        {
            InitEvent(&Event, cb->Data);
            va_start(Event.ArgList, Format);
            cb->Log(&Event);
            va_end(Event.ArgList);
        }
    }

    LogUnlock();
}
