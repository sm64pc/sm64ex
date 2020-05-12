#include <stdbool.h>
#include <string.h>
#include "discordrpc.h"

#define DISCORD_APP_ID  "709083908708237342"

int m_bErrored;
DiscordRichPresence m_sDiscordRichPresence;
bool m_bInitializeRequested;


void OnReady( const DiscordUser* user )
{
    discordReset();
}

void OnDiscordError(int errorCode, const char *szMessage)
{
    m_bErrored = true;
}


void InitializeDiscord()
{
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready          = OnReady;
    handlers.errored        = OnDiscordError;

    Discord_Initialize( DISCORD_APP_ID, &handlers, false, "" );
    discordReset();
}

void SetLogo()
{
    m_sDiscordRichPresence.largeImageKey = "head";
    //m_sDiscordRichPresence.largeImageText = "";
    //m_sDiscordRichPresence.smallImageKey = "";
    //m_sDiscordRichPresence.smallImageText = "";
}

void discordUpdateRichPresence()
{   

    SetLogo();

    Discord_UpdatePresence(&m_sDiscordRichPresence);
}

void discordShutdown()
{
    Discord_Shutdown();
};

void discordInit()
{
    InitializeDiscord();
    m_bInitializeRequested = true;
};

void discordReset()
{
    memset( &m_sDiscordRichPresence, 0, sizeof( m_sDiscordRichPresence ) );
    m_sDiscordRichPresence.details = "Super Mario 64 PC"; 
    // TODO set detail to current level name
    m_sDiscordRichPresence.state = "";

    SetLogo();
    Discord_UpdatePresence( &m_sDiscordRichPresence );
}