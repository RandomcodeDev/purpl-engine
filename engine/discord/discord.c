#include "engine/engine.h"

#include "discord.h"

static BOOLEAN Connected;
static CONST DiscordUser *User;
static UINT64 ApiCooldown;

static VOID Ready(CONST DiscordUser *NewUser)
{
    User = NewUser;
    Connected = TRUE;
    LogInfo("Discord connected %s#%s (%s)", User->username, User->discriminator, User->userId);
}

static VOID Disconnected(INT ErrorCode, PCSTR Message)
{
    Connected = FALSE;
    User = NULL;

    if (ErrorCode != 0)
    {
        LogError("Discord disconnected: %s (%d)", Message, ErrorCode);
    }
    else
    {
        LogInfo("Discord disconnected: %s", Message);
    }
}

static CONST UINT64 FriendIds[] = {570760243341033472};

static BOOLEAN IsFriend(UINT64 Id)
{
    for (SIZE_T i = 0; i < PURPL_ARRAYSIZE(FriendIds); i++)
    {
        if (FriendIds[i] == Id)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static PCSTR GetGameString()
{
    if (!User)
    {
        return "the game";
    }

    switch (atoll(User->userId))
    {
    case 532320702611587112:
        return "my game";
    case 405454975750373376:
        return "a game I helped my friend come up with";
    case 1078816552629051423:
        return "my brother's game";
    default:
        if (IsFriend(atoll(User->userId)))
        {
            return "my friend's game";
        }
        else
        {
            return "the game";
        }
    }
}

VOID DiscordInitialize(VOID)
{
    LogInfo("Attempting to initialize Discord");

    DiscordEventHandlers EventHandlers = {0};
    EventHandlers.ready = Ready;
    EventHandlers.disconnected = Disconnected;
    EventHandlers.errored = Disconnected;
    Discord_Initialize(PURPL_STRINGIZE_EXPAND(GAME_DISCORD_APP_ID), &EventHandlers, TRUE, NULL);
}

VOID DiscordUpdate()
{
    if (!User)
    {
        return;
    }

    DiscordRichPresence Presence = {0};
#ifdef PURPL_DEBUG
    Presence.state = CmnFormatString("Testing %s", GetGameString());
#else
    Presence.state = CmnFormatString("Playing %s", GetGameString());
#endif
    Presence.details = CmnFormatString("%s\nv%s commit %s-%s, %s build", PlatGetDescription(), GAME_VERSION_STRING,
                                       GAME_COMMIT, GAME_BRANCH,
#ifdef PURPL_DEBUG
                                       "debug"
#else
                                       "release"
#endif
    );
    Discord_UpdatePresence(&Presence);

    CmnFree(Presence.state);
    CmnFree(Presence.details);

    Discord_RunCallbacks();
}

VOID DiscordShutdown(VOID)
{
    Discord_ClearPresence();
    Discord_Shutdown();
    Connected = FALSE;
}

BOOLEAN DiscordConnected(VOID)
{
    return Connected;
}
