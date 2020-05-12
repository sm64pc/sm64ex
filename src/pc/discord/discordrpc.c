#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "PR/ultratypes.h"
#include "memory.h"
#include "discordrpc.h"

#define DISCORD_APP_ID  "709083908708237342"
#define DISCORD_UPDATE_RATE 5

time_t m_flLastUpdatedTime;
int m_bErrored;
DiscordRichPresence m_sDiscordRichPresence;
extern s16 gCurrCourseNum;
extern s16 gCurrActNum;
s16 lastCourseNum = 0;
s16 lastActNum = 0;
extern u8 seg2_course_name_table[];
extern u8 seg2_act_name_table[];

char stage[188];
char act[188];

char smallImageKey[5];
char largeImageKey[5];

char* chars[0xFF] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 7
    ' ', ' ', 'a', 'b', 'c', 'd', 'e', 'f', // 15
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', // 23
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', // 31
    'w', 'x', 'y', 'z', ' ', ' ', ' ', ' ', // 39
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 49
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 55
    ' ', ' ', ' ', ' ', ' ', ' ', '\'', ' ', // 63
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 71
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 79
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 87
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 95
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 103
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ',', // 111
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 119
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 127
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 135
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 143
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 151
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', '-', // 159
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 167
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 175
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 183
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 192
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 199
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 207
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 215
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 223
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 231
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 239
    ' ', ' ', '!', ' ', ' ', ' ', ' ', ' ', // 247
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '  // 255
};

void convertstring(const u8 *str, char* output)
{
    s32 strPos = 0;
    bool capitalizeChar = true;

    while (str[strPos] != 0xFF) 
    {
        if (str[strPos] < 0xFF)
        {
            output[strPos] = chars[str[strPos]];

            if (capitalizeChar && 0x0A <= str[strPos] && str[strPos] <= 0x23)
            {
                output[strPos] -= ('a' - 'A');
                capitalizeChar = false;
            }

        }
        else output[strPos] = ' ';

        if (output[strPos] == ' ' && str[strPos] != 158) fprintf(stdout, "Unknown Character (%i)\n", str[strPos]);
        if (output[strPos] == ' ') capitalizeChar = true;
        else capitalizeChar = false;
        strPos++;
    }

    output[strPos] = '\0';
}
 
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)
    n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

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

void SetDetails()
{
    if (lastCourseNum != gCurrCourseNum)
    {
        if (gCurrCourseNum)
        {
            void **courseNameTbl = segmented_to_virtual(seg2_course_name_table);
            u8 *courseName = segmented_to_virtual(courseNameTbl[gCurrCourseNum - 1]);

            convertstring(&courseName[3], stage);

            m_sDiscordRichPresence.details = stage;
        }
        else stage[0] = '\0';

        lastCourseNum = gCurrCourseNum;
    }
}

void SetState()
{
    if (lastActNum != gCurrActNum || lastCourseNum != gCurrCourseNum)
    {
        if (gCurrActNum && gCurrCourseNum)
        {
            if (gCurrCourseNum < 19) // any stage over 19 is a special stage without acts
            {
                void **actNameTbl = segmented_to_virtual(seg2_act_name_table);
                u8 *actName = actName = segmented_to_virtual(actNameTbl[(gCurrCourseNum - 1) * 6 + gCurrActNum - 1]);

                convertstring(actName, act);

                m_sDiscordRichPresence.state = act;
            }
            else
            {
                act[0] = '\0';
                gCurrActNum = 0;
            }
        }
        else act[0] = '\0';

        lastActNum = gCurrActNum;
    }
}

void SetLogo()
{
    if (lastCourseNum)
    {
        itoa(lastCourseNum, largeImageKey);
    }
    else strcpy(largeImageKey, "0");


    /*
    if (lastActNum)
    {
        itoa(lastActNum, smallImageKey);
    }
    else smallImageKey[0] = '\0';
    */

    m_sDiscordRichPresence.largeImageKey = largeImageKey;
    //m_sDiscordRichPresence.largeImageText = "";
    //m_sDiscordRichPresence.smallImageKey = smallImageKey;
    //m_sDiscordRichPresence.smallImageText = "";
}

void discordUpdateRichPresence()
{   
    if (time(NULL) < m_flLastUpdatedTime + DISCORD_UPDATE_RATE) return;

    m_flLastUpdatedTime = time(NULL);

    SetState();
    SetDetails();
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
    m_flLastUpdatedTime = 0;
};

void discordReset()
{
    memset( &m_sDiscordRichPresence, 0, sizeof( m_sDiscordRichPresence ) );

    SetState();
    SetDetails();
    SetLogo();
    Discord_UpdatePresence( &m_sDiscordRichPresence );
}