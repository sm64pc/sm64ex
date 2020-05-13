#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "PR/ultratypes.h"
#include "memory.h"
#include "pc/configfile.h"
#include "discordrpc.h"

#define DISCORD_APP_ID  "709083908708237342"
#define DISCORD_UPDATE_RATE 5

bool initialized = false;
time_t lastUpdatedTime;

DiscordRichPresence discordRichPresence;

extern s16 gCurrCourseNum;
extern s16 gCurrActNum;
s16 lastCourseNum = -1;
s16 lastActNum = -1;

extern u8 seg2_course_name_table[];
extern u8 seg2_act_name_table[];

#ifdef VERSION_EU
extern s32 gInGameLanguage;
#endif

char stage[188];
char act[188];

char smallImageKey[5];
char largeImageKey[5];

char charset[0xFF+1] = {
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
            output[strPos] = charset[str[strPos]];

            // if the char is a letter we can capatalize it
            if (capitalizeChar && 0x0A <= str[strPos] && str[strPos] <= 0x23)
            {
                output[strPos] -= ('a' - 'A');
                capitalizeChar = false;
            }

        }
        else output[strPos] = ' ';

        if (output[strPos] == ' ')
        {
            if (str[strPos] != 158) printf(stdout, "Unknown Character (%i)\n", str[strPos]); // inform that an unknown char was found
            capitalizeChar = true;
        }
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

    if (n < 0)
    n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    s[i] = '\0';
    reverse(s);
}

void OnReady( const DiscordUser* user )
{
    discordReset();
}

void InitializeDiscord()
{
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready          = OnReady;

    Discord_Initialize( DISCORD_APP_ID, &handlers, false, "" );
}

void SetDetails()
{
    if (lastCourseNum != gCurrCourseNum)
    {
        // If we are in in Course 0 we are in the castle which doesn't have a string
        if (gCurrCourseNum)
        {
            void **courseNameTbl;

#ifndef VERSION_EU
            courseNameTbl = segmented_to_virtual(seg2_course_name_table);
#else
            switch (gInGameLanguage) {
                case LANGUAGE_ENGLISH:
                    courseNameTbl = segmented_to_virtual(course_name_table_eu_en);
                    break;
                case LANGUAGE_FRENCH:
                    courseNameTbl = segmented_to_virtual(course_name_table_eu_fr);
                    break;
                case LANGUAGE_GERMAN:
                    courseNameTbl = segmented_to_virtual(course_name_table_eu_de);
                    break;
            }
#endif
            u8 *courseName = segmented_to_virtual(courseNameTbl[gCurrCourseNum - 1]);

            convertstring(&courseName[3], stage);
        }
        else strcpy(stage, "Peach's Castle");

        lastCourseNum = gCurrCourseNum;
    }
}

void SetState()
{
    if (lastActNum != gCurrActNum || lastCourseNum != gCurrCourseNum)
    {
        // when exiting a stage the act doesn't get reset
        if (gCurrActNum && gCurrCourseNum)
        {
            if (gCurrCourseNum < 19) // any stage over 19 is a special stage without acts
            {
                void **actNameTbl;
#ifndef VERSION_EU
                actNameTbl = segmented_to_virtual(seg2_act_name_table);
#else
                switch (gInGameLanguage) {
                    case LANGUAGE_ENGLISH:
                        actNameTbl = segmented_to_virtual(act_name_table_eu_en);
                        break;
                    case LANGUAGE_FRENCH:
                        actNameTbl = segmented_to_virtual(act_name_table_eu_fr);
                        break;
                    case LANGUAGE_GERMAN:
                        actNameTbl = segmented_to_virtual(act_name_table_eu_de);
                        break;
                }
#endif
                u8 *actName = actName = segmented_to_virtual(actNameTbl[(gCurrCourseNum - 1) * 6 + gCurrActNum - 1]);

                convertstring(actName, act);
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

    discordRichPresence.largeImageKey = largeImageKey;
    //discordRichPresence.largeImageText = "";
    //discordRichPresence.smallImageKey = smallImageKey;
    //discordRichPresence.smallImageText = "";
}

void discordUpdateRichPresence()
{   
    if (initialized && time(NULL) < lastUpdatedTime + DISCORD_UPDATE_RATE) return;

    lastUpdatedTime = time(NULL);

    SetState();
    SetDetails();
    SetLogo();
    Discord_UpdatePresence(&discordRichPresence);
}

void discordShutdown()
{
    Discord_Shutdown();
};

void discordInit()
{
    if (configDiscordRPC)
    {
        InitializeDiscord();

        discordRichPresence.details = stage;
        discordRichPresence.state = act;


        lastUpdatedTime = 0;
        initialized = true;
    }
};

void discordReset()
{
    memset( &discordRichPresence, 0, sizeof( discordRichPresence ) );

    SetState();
    SetDetails();
    SetLogo();
    Discord_UpdatePresence( &discordRichPresence );
}