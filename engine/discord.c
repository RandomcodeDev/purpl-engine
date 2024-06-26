#include "engine/engine.h"
#include "engine/render/render.h"

#include "discord.h"

static BOOLEAN Connected;
static DiscordUser User;

static VOID Ready(CONST DiscordUser *NewUser)
{
    User = *NewUser;
    Connected = TRUE;
    LogDebug("Discord connected as %s (%s)", User.username, User.userId);
}

static VOID Disconnected(INT ErrorCode, PCSTR Message)
{
    Connected = FALSE;
    memset(&User, 0, sizeof(DiscordUser));

    if (Message && ErrorCode != 0)
    {
        LogError("Discord disconnected: %s (%d)", Message, ErrorCode);
    }
    else
    {
        LogDebug("Discord disconnected%s%s", Message ? ": " : "", Message ? Message : "");
    }
}

static CONST UINT64 FriendIds[] = {570760243341033472, 802941540120789014, 691017722934329394, 436582998171844608,
                                   744607381991718913, 526963318867492865, 878017826558996570, 898953887988482068,
                                   751186390598811739, 700010638730330122, 887865846414868521, 551486661079334912,
                                   344253142952574989, 343862296751112192, 515919551444025407, 725427722293215332,
                                   464268944459563018};

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
    UINT64 UserId = strtoll(User.userId ? User.userId : "", NULL, 10);
    switch (UserId)
    {
    case 532320702611587112:
        return "my game";
    case 405454975750373376:
        return "a game I helped my friend come up with";
    case 1078816552629051423:
        return "my brother's game";
    default:
        if (IsFriend(UserId))
        {
            return "my friend's game";
        }
        else
        {
            return "the game"; // you just lost the game
        }
    }
}

VOID DiscordInitialize(_In_ ecs_iter_t *Iterator)
{
    UNREFERENCED_PARAMETER(Iterator);

    LogInfo("Attempting to initialize Discord");

    DiscordEventHandlers EventHandlers = {0};
    EventHandlers.ready = Ready;
    EventHandlers.disconnected = Disconnected;
    EventHandlers.errored = Disconnected;
    Discord_Initialize(PURPL_STRINGIZE_EXPAND(PURPL_DISCORD_APP_ID), &EventHandlers, TRUE, NULL);
}
ecs_entity_t ecs_id(DiscordInitialize);

VOID DiscordUpdate(_In_ ecs_iter_t *Iterator)
{
    UNREFERENCED_PARAMETER(Iterator);

    DiscordRichPresence Presence = {0};
#ifdef PURPL_DEBUG
    Presence.state = CmnFormatString("Testing %s on %s", GetGameString(), PlatGetDescription());
#else
    Presence.state = CmnFormatString("Playing %s on %s", GetGameString(), PlatGetDescription());
#endif
    Presence.details = CmnFormatString("v" PURPL_VERSION_STRING " commit " PURPL_COMMIT "-" PURPL_BRANCH
                                       ", " PURPL_BUILD_TYPE " build, %s on %s",
                                       RdrGetApiName(CONFIGVAR_GET_INT("rdr_api")), RdrGetGpuName());
    Discord_UpdatePresence(&Presence);

    CmnFree(Presence.state);
    CmnFree(Presence.details);

    Discord_RunCallbacks();
}
ecs_entity_t ecs_id(DiscordUpdate);

VOID DiscordImport(_In_ ecs_world_t *World)
{
    LogTrace("Importing Discord ECS module");

    ECS_MODULE(World, Discord);

    ECS_SYSTEM_DEFINE(World, DiscordInitialize, EcsOnStart);
    ECS_SYSTEM_DEFINE_EX(World, DiscordUpdate, EcsOnUpdate, false, DISCORD_UPDATE_INTERVAL);
}

VOID DiscordShutdown(VOID)
{
    Discord_ClearPresence();
    Discord_Shutdown();
    Disconnected(0, NULL);
}

BOOLEAN DiscordConnected(VOID)
{
    return Connected;
}
