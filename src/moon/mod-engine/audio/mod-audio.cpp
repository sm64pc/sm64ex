#include "mod-audio.h"

#include "moon/fs/moonfs.h"
#include "moon/libs/nlohmann/json.hpp"
#include "moon/mod-engine/engine.h"
#include "moon/mod-engine/hooks/hook.h"
#include "moon/mod-engine/interfaces/sound-entry.h"

extern "C" {
#include "text/libs/io_utils.h"
#include "pc/platform.h"
#include "pc/fs/fs.h"
#include "PR/libaudio.h"
#include "audio/load.h"
#include "audio/external.h"
}

#include <iostream>
#include <vector>
#include <map>

using namespace std;
using json = nlohmann::json;

u8 backgroundQueueSize = 0;


vector<BitModule*> soundCache;

namespace Moon {
    void saveAddonSound(BitModule *addon, std::string soundPath, EntryFileData* data){
        if(addon->sounds == nullptr)
            addon->sounds = new SoundEntry();

        SoundEntry* soundEntry = addon->sounds;
        ALSeqFile* header = (ALSeqFile*) data->data;

        if(soundPath == "sound/sequences.bin"){
            soundEntry->seqHeader = header;
            alSeqFileNew(soundEntry->seqHeader, (u8*) data->data);
            soundEntry->seqCount = header->seqCount;
            return;
        }
        if(soundPath == "sound/sound_data.ctl"){
            soundEntry->ctlHeader = header;
            alSeqFileNew(soundEntry->ctlHeader, (u8*) data->data);
            soundEntry->ctlEntries = new CtlEntry[header->seqCount];
            return;
        }
        if(soundPath == "sound/sound_data.tbl"){
            soundEntry->tblHeader = header;
            alSeqFileNew(soundEntry->tblHeader, (u8*) data->data);
            return;
        }
        if(soundPath == "sound/bank_sets"){
            soundEntry->bankSets = (u8*) data->data;
            return;
        }
    }
}

namespace Moon {
    void setSoundEntry(SoundEntry *entry) {
        gSeqFileHeader = entry->seqHeader;
        gSequenceCount = entry->seqCount;

        gAlCtlHeader   = entry->ctlHeader;
        gCtlEntries    = entry->ctlEntries;

        gAlTbl         = entry->tblHeader;
        gAlBankSets    = entry->bankSets;
    }
}

namespace MoonInternal {

    void buildAudioCache(vector<int> order){
        soundCache.clear();

        for(int i=0; i < order.size(); i++){
            soundCache.push_back(Moon::addons[order[i]]);
        }

        Moon::setSoundEntry(soundCache[soundCache.size() - 1]->sounds);
    }

    void resetSound(){
        backgroundQueueSize = sBackgroundMusicQueueSize;

    }

    void setupSoundEngine( string state ){
        if(state == "Exit"){
            for(auto &addon : Moon::addons){
                delete addon->sounds;
            }
        }
    }
}