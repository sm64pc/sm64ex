#include "moon/config/mooncfg.h"

extern "C" {
#include "buffers/buffers.h"
}

MoonCFG *cfg;

using namespace std;

namespace Moon {
    void readSaveFile(int fileIndex){
        cfg = new MoonCFG("moon64_save_file_"+to_string(fileIndex)+".sav");
        gSaveBuffer.menuData[0].soundMode = cfg->getInt("SoundMode");
        gSaveBuffer.menuData[0].coinScoreAges[fileIndex] = cfg->getInt("CoinScoreAge");
    }
}
