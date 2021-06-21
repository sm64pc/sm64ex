#include "moon/config/mooncfg.h"
#include <iostream>
#include <vector>
#include <map>
#include <filesystem>
#include "moon/utils/moon-env.h"
#include "moon/achievements/achievements.h"
namespace fs = std::filesystem;

using namespace std;

extern "C" {
#include "buffers/buffers.h"
#include "game/save_file.h"
#include "game/text_save.h"
#include "pc/platform.h"
}

map<int, MoonCFG*> saveCache;

vector<string> sav_flags = {
    "file_exists", "wing_cap", "metal_cap", "vanish_cap", "key_1", "key_2",
    "basement_door", "upstairs_door", "ddd_moved_back", "moat_drained",
    "pps_door", "wf_door", "ccm_door", "jrb_door", "bitdw_door",
    "bitfs_door", "", "", "", "", "50star_door"
};

vector<string> sav_courses = {
    "bob", "wf", "jrb", "ccm", "bbh", "hmc", "lll",
    "ssl", "ddd", "sl", "wdw", "ttm", "thi", "ttc", "rr"
};

vector<string> sav_bonus_courses = {
    "bitdw", "bitfs", "bits", "pss", "cotmc",
    "totwc", "vcutm", "wmotr", "sa", "hub"
};
vector<string> cap_on_types = {
    "ground", "klepto", "ukiki", "mrblizzard"
};

vector<string> sound_modes = {
    "stereo", "headset", "unk", "mono"
};

static u32 int_to_bin(u32 n) {
    s32 bin = 0, rem, i = 1;
    while (n != 0) {
        rem = n % 2;
        n /= 2;
        bin += rem * i;
        i *= 10;
    }
    return bin;
}

static u32 bin_to_int(u32 n) {
    s32 dec = 0, i = 0, rem;
    while (n != 0) {
        rem = n % 10;
        n /= 10;
        dec += rem * (1 << i);
        ++i;
    }
    return dec;
}

namespace MoonInternal {
    MoonCFG* getSaveFile(int fileIndex){
        if(saveCache.find(fileIndex) == saveCache.end()){
        #ifndef TARGET_SWITCH
            string cwd = MoonInternal::getEnvironmentVar("MOON_UPATH");
            string path = cwd.substr(0, cwd.find_last_of("/\\")) + "/moon64/Moon64-Save-"+to_string(fileIndex + 1)+".dat";
        #else
            string path = "sdmc:/moon64/Moon64-Save-"+to_string(fileIndex + 1)+".dat";
        #endif
            saveCache[fileIndex] = new MoonCFG(path, false);
        #ifdef GAME_DEBUG
            cout << "Loading save file: " << path << endl;
        #endif
        }
        return saveCache[fileIndex];
    }

    void writeSaveFile(int fileIndex){
        MoonCFG *cfg = getSaveFile(fileIndex);
        SaveFile *savedata = &gSaveBuffer.files[fileIndex][0];
        MainMenuSaveData *menudata = &gSaveBuffer.menuData[0];

        // Global Config
        cfg->setString("global.sound_mode",     sound_modes[menudata->soundMode]);
        cfg->setInt   ("global.coin_score_age", menudata->coinScoreAges[fileIndex]);

        for (int i = 0; i < sav_flags.size(); i++) {
            if(sav_flags[i] != ""){
                int bit = (1 << i);
                int flag = (save_file_get_flags() & bit);
                cfg->setBool("game.flags."+sav_flags[i], flag);
            }
        }

        for(int i = 0; i < sav_courses.size(); i++){
            string courseKey = "game.courses."+sav_courses[i];
            cfg->setInt (courseKey + ".stars",  int_to_bin(save_file_get_star_flags(fileIndex, i)));
            cfg->setInt (courseKey + ".coins",  save_file_get_course_coin_score(fileIndex, i));
            cfg->setBool(courseKey + ".cannon", save_file_get_cannon_flags(fileIndex, i));
        }

        int bonusCoursesSize = sav_bonus_courses.size();

        for(int i = 0; i < bonusCoursesSize; i++){
            int starFlag  = (i == bonusCoursesSize - 1) ? -1 : (i == 3) ? 18 : i + 15;
            string courseKey = "game.courses."+sav_bonus_courses[i];
            cfg->setInt(courseKey + ".stars", int_to_bin(save_file_get_star_flags(fileIndex, starFlag)));
        }

        for (int i = 0; i < cap_on_types.size(); i++) {
            int bit = (1 << (i+16));
            int flag = (save_file_get_flags() & bit);
            string capKey = "game.caps."+cap_on_types[i];
            cfg->setBool(capKey + ".unlocked", flag);
        }

        int lvl = savedata->capLevel;
        if(lvl == COURSE_SSL || lvl == COURSE_SL || COURSE_TTM)
            cfg->setString("game.caps.lost", sav_courses[lvl]);

    #ifndef GAME_DEBUG
        vector<string> obtainedAchievements;

        for( auto &rAchievements : entries[fileIndex] ){
            obtainedAchievements.push_back(rAchievements->achievement->id);
        }

        if(cheatsGotEnabled) obtainedAchievements.clear();

        cfg->setArray<string>("game.achievements", obtainedAchievements);
        cfg->setBool("game.updated", cheatsGotEnabled);
    #endif
        memcpy(&gSaveBuffer.files[fileIndex][1], &gSaveBuffer.files[fileIndex][0], sizeof(gSaveBuffer.files[fileIndex][1]));

        cfg->save();
    }

    template< typename T > int getItemIndex(std::vector<T> array, T object) {
        auto it = find(array.begin(), array.end(), object);
        return it != array.end() ? it - array.begin() : 0;
    };

    void readSaveFile(int fileIndex){
        MoonCFG *cfg = getSaveFile(fileIndex);
        if(cfg->isNewInstance) return;

        gSaveBuffer.menuData[0].soundMode = getItemIndex<string>(sound_modes, sound_modes[cfg->getInt("global.sound_mode")]);
        gSaveBuffer.menuData[0].coinScoreAges[fileIndex] = cfg->getInt("global.coin_score_age");

        for (int i = 0; i < sav_flags.size(); i++) {
            if(sav_flags[i] != ""){
                if(cfg->getBool("game.flags."+sav_flags[i]))
                    gSaveBuffer.files[fileIndex][0].flags |= ( 1 << i );
            }
        }

        for(int i = 0; i < sav_courses.size(); i++){
            string courseKey = "game.courses."+sav_courses[i];
            int cannon = int(cfg->getBool(courseKey + ".cannon"));
            save_file_set_star_flags(fileIndex, i, bin_to_int(cfg->getInt(courseKey + ".stars")));
            save_file_set_star_flags(fileIndex, i + 1, cannon <<= 7);
            gSaveBuffer.files[fileIndex][0].courseCoinScores[i] = cfg->getInt(courseKey + ".coins");
        }

        for(int i = 0; i < sav_bonus_courses.size(); i++){
            int starFlag  = (i == sav_bonus_courses.size() - 1) ? -1 : (i == 3) ? 18 : i + 15;
            string courseKey = "game.courses."+sav_bonus_courses[i];
            save_file_set_star_flags(fileIndex, starFlag, bin_to_int(cfg->getInt(courseKey + ".stars")));
        }

        for (int i = 0; i < cap_on_types.size(); i++) {
            if(cfg->getBool("game.caps."+cap_on_types[i]+".unlocked")){
                gSaveBuffer.files[fileIndex][0].flags |= (1 << (16 + i));
            }
        }

        if(cfg->contains("game.caps.lost"))
            gSaveBuffer.files[fileIndex][0].capArea = cfg->getInt("game.caps.lost");

    #ifndef GAME_DEBUG
        entries.clear();
        cheatsGotEnabled = cfg->getBool("game.updated");

        vector<string> obtainedAchievements = cfg->getArray<string>("game.achievements");

        for( auto &achievement : obtainedAchievements ){
            AchievementEntry* entry = new AchievementEntry();
            Achievement* a = Moon::getAchievementById(achievement);
            if(a == NULL) return;
            entry->achievement = a;
            entry->dead = true;
            entries[fileIndex].push_back(entry);
        }

        if(cheatsGotEnabled) entries.clear();
    #endif

        gSaveBuffer.files[fileIndex][0].flags |= (1 << 0);

        // Backup is nessecary for saving recent progress after gameover
        memcpy(&gSaveBuffer.files[fileIndex][1], &gSaveBuffer.files[fileIndex][0], sizeof(gSaveBuffer.files[fileIndex][1]));
    }

    void eraseSaveFile( int fileIndex ){
    #ifndef TARGET_SWITCH
        string cwd = MoonInternal::getEnvironmentVar("MOON_UPATH");
        string path = cwd.substr(0, cwd.find_last_of("/\\")) + "/moon64/Moon64-Save-"+to_string(fileIndex + 1)+".dat";
    #else
        string path = "sdmc:/moon64/Moon64-Save-"+to_string(fileIndex + 1)+".dat";
    #endif
        if(fs::exists(path))
            fs::remove(path);
    }

    void setupSaveEngine(string state){
        if(state == "PreInit"){
            // Scan old save format
        #ifndef TARGET_SWITCH
            string cwd = MoonInternal::getEnvironmentVar("MOON_UPATH");
            string path = cwd.substr(0, cwd.find_last_of("/\\")) + "/moon64/";
        #else
            string path = "sdmc:/";
        #endif

            vector<string> supportedSaves = {
                "moon64_save_file_", "sm64_save_file_",
            };

            for (const auto & entry : fs::directory_iterator(path)){
                string file = entry.path().string();
                auto idx = file.rfind('.');
                if(idx != std::string::npos) {
                    if(file.substr(idx + 1) == "sav"){
                        for(auto &s : supportedSaves){
                            size_t nameIdx = file.find(s);
                            if(nameIdx != std::string::npos){
                                int i = stoi(file.substr(nameIdx + s.length(), 1));
                                string fPath = file.substr(nameIdx);
                                cout << "Detected old format save file: " << fPath << endl;
                                cout << "Converting save file" << endl;
                                read_text_save(i, const_cast<char *>( fPath.c_str()));
                                writeSaveFile(i);
                                fs::rename(file, file+".old");
                                cout << "Done!" << endl;
                            }
                        }
                    }
                }
            }
            return;
        }
    }
}

extern "C"{
    void writeSaveFile(int saveIndex){
        MoonInternal::writeSaveFile(saveIndex);
    }
    void readSaveFile(int saveIndex){
        MoonInternal::readSaveFile(saveIndex);
    }
    void eraseSaveFile(int fileIndex){
        MoonInternal::eraseSaveFile(fileIndex);
    }
}
