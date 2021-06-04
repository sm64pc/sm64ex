#include "mod-audio.h"

#include "moon/fs/moonfs.h"
#include "moon/libs/nlohmann/json.hpp"
#include "moon/mod-engine/engine.h"
#include "moon/mod-engine/hooks/hook.h"

extern "C" {
#include "text/libs/io_utils.h"
#include "pc/platform.h"
#include "pc/fs/fs.h"
}

#include <iostream>
#include <vector>
#include <map>

using namespace std;
using json = nlohmann::json;

map<string, BitModule*> soundCache;

namespace Moon {
    void saveAddonSound(BitModule *addon, std::string soundPath, EntryFileData* data){
        addon->sounds.insert(pair<string, EntryFileData*>(soundPath, data));
    }
}

namespace MoonInternal {

    EntryFileData *getSoundData(const char *fullpath){
        char *actualname = sys_strdup(fullpath);

        auto cacheEntry = soundCache.find(actualname);

        if(cacheEntry == soundCache.end()) {
            cout << "Failed to read sound file" << fullpath << endl;
        }

        BitModule *addon = cacheEntry->second;

        EntryFileData * data = NULL;

        if(addon != NULL){
            EntryFileData *fileEntry = addon->sounds.find(actualname)->second;

            if(fileEntry != NULL){
                if(fileEntry->data != NULL) data = fileEntry;
                else if(!fileEntry->path.empty()){
                    MoonFS file(addon->path);
                    file.open();
                    EntryFileData *newData = new EntryFileData();
                    file.read(fileEntry->path, newData);
                    data = newData;
                }
            }
        }
        return data;
    }

    void buildAudioCache(vector<int> order){
        soundCache.clear();

        for(int i=0; i < order.size(); i++){
            BitModule *addon = Moon::addons[order[i]];

            for (map<string, EntryFileData*>::iterator entry = addon->sounds.begin(); entry != addon->sounds.end(); ++entry)
                soundCache.insert(pair<string, BitModule*>(entry->first, addon));
        }
    }

    void setupSoundEngine( string state ){
        if(state == "Exit"){
            for(auto &addon : Moon::addons){
                addon->sounds.clear();
            }
        }
    }
}

extern "C" {
void* loadSoundData(const char* fullpath){
    EntryFileData *data = MoonInternal::getSoundData(fullpath);
    if(data != NULL)
        return data->data;
    return NULL;
}
}