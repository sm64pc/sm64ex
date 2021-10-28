#include "saturn_colors.h"
#include "saturn.h"
#include "moon/mod-engine/hooks/hook.h"

#include "moon/utils/moon-env.h"
#include "moon/fs/moonfs.h"
#include "pc/configfile.h"
#include "moon/imgui/imgui_impl.h"
#include "moon/libs/imgui/imgui.h"
#include "moon/libs/imgui/imgui_internal.h"
#include "moon/libs/imgui/imgui_impl_sdl.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include <dirent.h>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

extern "C" {
#include "game/camera.h"
#include "game/level_update.h"
#include "sm64.h"
}

unsigned int defaultColorHatRLight = 255;
unsigned int defaultColorHatRDark = 127;
unsigned int defaultColorHatGLight = 0;
unsigned int defaultColorHatGDark = 0;
unsigned int defaultColorHatBLight = 0;
unsigned int defaultColorHatBDark = 0;

unsigned int defaultColorOverallsRLight = 0;
unsigned int defaultColorOverallsRDark = 0;
unsigned int defaultColorOverallsGLight = 0;
unsigned int defaultColorOverallsGDark = 0;
unsigned int defaultColorOverallsBLight = 255;
unsigned int defaultColorOverallsBDark = 127;

unsigned int defaultColorGlovesRLight = 255;
unsigned int defaultColorGlovesRDark = 127;
unsigned int defaultColorGlovesGLight = 255;
unsigned int defaultColorGlovesGDark = 127;
unsigned int defaultColorGlovesBLight = 255;
unsigned int defaultColorGlovesBDark = 127;

unsigned int defaultColorShoesRLight = 114;
unsigned int defaultColorShoesRDark = 57;
unsigned int defaultColorShoesGLight = 28;
unsigned int defaultColorShoesGDark = 14;
unsigned int defaultColorShoesBLight = 14;
unsigned int defaultColorShoesBDark = 7;

unsigned int defaultColorSkinRLight = 254;
unsigned int defaultColorSkinRDark = 127;
unsigned int defaultColorSkinGLight = 193;
unsigned int defaultColorSkinGDark = 96;
unsigned int defaultColorSkinBLight = 121;
unsigned int defaultColorSkinBDark = 60;

unsigned int defaultColorHairRLight = 115;
unsigned int defaultColorHairRDark = 57;
unsigned int defaultColorHairGLight = 6;
unsigned int defaultColorHairGDark = 3;
unsigned int defaultColorHairBLight = 0;
unsigned int defaultColorHairBDark = 0;

// Color Codes

namespace MoonInternal {

    std::vector<string> cc_array;
    string colorCodeDir;

    void load_cc_directory() {
        cc_array.clear();
        cc_array.push_back("Mario.gs");

        string cwd = MoonInternal::getEnvironmentVar("MOON_CWD");
#ifdef __MINGW32__
        // Windows moment
        colorCodeDir = cwd.substr(0, cwd.find_last_of("/\\")) + "\\addons\\saturn\\assets\\colorcodes\\";
#else
        colorCodeDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/addons/saturn/assets/colorcodes/";
#endif

        for (const auto & entry : fs::directory_iterator(colorCodeDir))
            cc_array.push_back(entry.path().filename().u8string());

        //std::cout << cc_array[0] << std::endl;
    }

    void reset_cc_colors() {
        defaultColorHatRLight = 255;
        defaultColorHatRDark = 127;
        defaultColorHatGLight = 0;
        defaultColorHatGDark = 0;
        defaultColorHatBLight = 0;
        defaultColorHatBDark = 0;

        defaultColorOverallsRLight = 0;
        defaultColorOverallsRDark = 0;
        defaultColorOverallsGLight = 0;
        defaultColorOverallsGDark = 0;
        defaultColorOverallsBLight = 255;
        defaultColorOverallsBDark = 127;

        defaultColorGlovesRLight = 255;
        defaultColorGlovesRDark = 127;
        defaultColorGlovesGLight = 255;
        defaultColorGlovesGDark = 127;
        defaultColorGlovesBLight = 255;
        defaultColorGlovesBDark = 127;

        defaultColorShoesRLight = 114;
        defaultColorShoesRDark = 57;
        defaultColorShoesGLight = 28;
        defaultColorShoesGDark = 14;
        defaultColorShoesBLight = 14;
        defaultColorShoesBDark = 7;

        defaultColorSkinRLight = 254;
        defaultColorSkinRDark = 127;
        defaultColorSkinGLight = 193;
        defaultColorSkinGDark = 96;
        defaultColorSkinBLight = 121;
        defaultColorSkinBDark = 60;

        defaultColorHairRLight = 115;
        defaultColorHairRDark = 57;
        defaultColorHairGLight = 6;
        defaultColorHairGDark = 3;
        defaultColorHairBLight = 0;
        defaultColorHairBDark = 0;

        enable_cap_logo = true;
    }

    void load_cc_file(string cc_filename) {
        if (cc_filename == "Mario.gs") {
            reset_cc_colors();
            return;
        }

        std::ifstream file(colorCodeDir + cc_filename, std::ios::in | std::ios::binary);

        // If the color code was previously deleted, reload the list and cancel.
        if (!file.good()) {
            load_cc_directory();
            return;
        }

        const std::size_t& size = std::filesystem::file_size(colorCodeDir + cc_filename);
        std::string content(size, '\0');
        file.read(content.data(), size);

        file.close();

        std::istringstream f(content);
        std::string line;
        
        while (std::getline(f, line)) {
            std::string address = line.substr(2, 6);
            int value1 = std::stoi(line.substr(9, 2), 0, 16);
            int value2 = std::stoi(line.substr(11, 2), 0, 16);

            // Hat
            if (address == "07EC40") {
                defaultColorHatRLight = value1;
                defaultColorHatGLight = value2;
            }
            if (address == "07EC42") {
                defaultColorHatBLight = value1;
            }
            if (address == "07EC38") {
                defaultColorHatRDark = value1;
                defaultColorHatGDark = value2;
            }
            if (address == "07EC3A") {
                defaultColorHatBDark = value1;
            }

            // Overalls
            if (address == "07EC28") {
                defaultColorOverallsRLight = value1;
                defaultColorOverallsGLight = value2;
            }
            if (address == "07EC2A") {
                defaultColorOverallsBLight = value1;
            }
            if (address == "07EC20") {
                defaultColorOverallsRDark = value1;
                defaultColorOverallsGDark = value2;
            }
            if (address == "07EC22") {
                defaultColorOverallsBDark = value1;
            }

            // Gloves
            if (address == "07EC58") {
                defaultColorGlovesRLight = value1;
                defaultColorGlovesGLight = value2;
            }
            if (address == "07EC5A") {
                defaultColorGlovesBLight = value1;
            }
            if (address == "07EC50") {
                defaultColorGlovesRDark = value1;
                defaultColorGlovesGDark = value2;
            }
            if (address == "07EC52") {
                defaultColorGlovesBDark = value1;
            }

            // Shoes
            if (address == "07EC70") {
                defaultColorShoesRLight = value1;
                defaultColorShoesGLight = value2;
            }
            if (address == "07EC72") {
                defaultColorShoesBLight = value1;
            }
            if (address == "07EC68") {
                defaultColorShoesRDark = value1;
                defaultColorShoesGDark = value2;
            }
            if (address == "07EC6A") {
                defaultColorShoesBDark = value1;
            }

            // Skin
            if (address == "07EC88") {
                defaultColorSkinRLight = value1;
                defaultColorSkinGLight = value2;
            }
            if (address == "07EC8A") {
                defaultColorSkinBLight = value1;
            }
            if (address == "07EC80") {
                defaultColorSkinRDark = value1;
                defaultColorSkinGDark = value2;
            }
            if (address == "07EC82") {
                defaultColorSkinBDark = value1;
            }

            // Hair
            if (address == "07ECA0") {
                defaultColorHairRLight = value1;
                defaultColorHairGLight = value2;
            }
            if (address == "07ECA2") {
                defaultColorHairBLight = value1;
            }
            if (address == "07EC98") {
                defaultColorHairRDark = value1;
                defaultColorHairGDark = value2;
            }
            if (address == "07EC9A") {
                defaultColorHairBDark = value1;
            }

            enable_cap_logo = false;
        }
    }

    void save_cc_file(std::string name) {
        std::string gameshark;

        char col1char[64];
        ImFormatString(col1char, IM_ARRAYSIZE(col1char), "%02X%02X%02X", ImClamp((int)defaultColorHatRLight, 0, 255), ImClamp((int)defaultColorHatGLight, 0, 255), ImClamp((int)defaultColorHatBLight, 0, 255));
        std::string col1 = col1char;
        char col2char[64];
        ImFormatString(col2char, IM_ARRAYSIZE(col2char), "%02X%02X%02X", ImClamp((int)defaultColorHatRDark, 0, 255), ImClamp((int)defaultColorHatGDark, 0, 255), ImClamp((int)defaultColorHatBDark, 0, 255));
        std::string col2 = col2char;
        char col3char[64];
        ImFormatString(col3char, IM_ARRAYSIZE(col3char), "%02X%02X%02X", ImClamp((int)defaultColorOverallsRLight, 0, 255), ImClamp((int)defaultColorOverallsGLight, 0, 255), ImClamp((int)defaultColorOverallsBLight, 0, 255));
        std::string col3 = col3char;
        char col4char[64];
        ImFormatString(col4char, IM_ARRAYSIZE(col4char), "%02X%02X%02X", ImClamp((int)defaultColorOverallsRDark, 0, 255), ImClamp((int)defaultColorOverallsGDark, 0, 255), ImClamp((int)defaultColorOverallsBDark, 0, 255));
        std::string col4 = col4char;
        char col5char[64];
        ImFormatString(col5char, IM_ARRAYSIZE(col5char), "%02X%02X%02X", ImClamp((int)defaultColorGlovesRLight, 0, 255), ImClamp((int)defaultColorGlovesGLight, 0, 255), ImClamp((int)defaultColorGlovesBLight, 0, 255));
        std::string col5 = col5char;
        char col6char[64];
        ImFormatString(col6char, IM_ARRAYSIZE(col6char), "%02X%02X%02X", ImClamp((int)defaultColorGlovesRDark, 0, 255), ImClamp((int)defaultColorGlovesGDark, 0, 255), ImClamp((int)defaultColorGlovesBDark, 0, 255));
        std::string col6 = col6char;
        char col7char[64];
        ImFormatString(col7char, IM_ARRAYSIZE(col7char), "%02X%02X%02X", ImClamp((int)defaultColorShoesRLight, 0, 255), ImClamp((int)defaultColorShoesGLight, 0, 255), ImClamp((int)defaultColorShoesBLight, 0, 255));
        std::string col7 = col7char;
        char col8char[64];
        ImFormatString(col8char, IM_ARRAYSIZE(col8char), "%02X%02X%02X", ImClamp((int)defaultColorShoesRDark, 0, 255), ImClamp((int)defaultColorShoesGDark, 0, 255), ImClamp((int)defaultColorShoesBDark, 0, 255));
        std::string col8 = col8char;
        char col9char[64];
        ImFormatString(col9char, IM_ARRAYSIZE(col9char), "%02X%02X%02X", ImClamp((int)defaultColorSkinRLight, 0, 255), ImClamp((int)defaultColorSkinGLight, 0, 255), ImClamp((int)defaultColorSkinBLight, 0, 255));
        std::string col9 = col9char;
        char col10char[64];
        ImFormatString(col10char, IM_ARRAYSIZE(col10char), "%02X%02X%02X", ImClamp((int)defaultColorSkinRDark, 0, 255), ImClamp((int)defaultColorSkinGDark, 0, 255), ImClamp((int)defaultColorSkinBDark, 0, 255));
        std::string col10 = col10char;
        char col11char[64];
        ImFormatString(col11char, IM_ARRAYSIZE(col11char), "%02X%02X%02X", ImClamp((int)defaultColorHairRLight, 0, 255), ImClamp((int)defaultColorHairGLight, 0, 255), ImClamp((int)defaultColorHairBLight, 0, 255));
        std::string col11 = col11char;
        char col12char[64];
        ImFormatString(col12char, IM_ARRAYSIZE(col12char), "%02X%02X%02X", ImClamp((int)defaultColorHairRDark, 0, 255), ImClamp((int)defaultColorHairGDark, 0, 255), ImClamp((int)defaultColorHairBDark, 0, 255));
        std::string col12 = col12char;

        gameshark += "8107EC40 " + col1.substr(0, 2) + col1.substr(2, 2) + "\n";
        gameshark += "8107EC42 " + col1.substr(4, 2) + "00\n";
        gameshark += "8107EC38 " + col2.substr(0, 2) + col2.substr(2, 2) + "\n";
        gameshark += "8107EC3A " + col2.substr(4, 2) + "00\n";
        gameshark += "8107EC28 " + col3.substr(0, 2) + col3.substr(2, 2) + "\n";
        gameshark += "8107EC2A " + col3.substr(4, 2) + "00\n";
        gameshark += "8107EC20 " + col4.substr(0, 2) + col4.substr(2, 2) + "\n";
        gameshark += "8107EC22 " + col4.substr(4, 2) + "00\n";
        gameshark += "8107EC58 " + col5.substr(0, 2) + col5.substr(2, 2) + "\n";
        gameshark += "8107EC5A " + col5.substr(4, 2) + "00\n";
        gameshark += "8107EC50 " + col6.substr(0, 2) + col6.substr(2, 2) + "\n";
        gameshark += "8107EC52 " + col6.substr(4, 2) + "00\n";
        gameshark += "8107EC70 " + col7.substr(0, 2) + col7.substr(2, 2) + "\n";
        gameshark += "8107EC72 " + col7.substr(4, 2) + "00\n";
        gameshark += "8107EC68 " + col8.substr(0, 2) + col8.substr(2, 2) + "\n";
        gameshark += "8107EC6A " + col8.substr(4, 2) + "00\n";
        gameshark += "8107EC88 " + col9.substr(0, 2) + col9.substr(2, 2) + "\n";
        gameshark += "8107EC8A " + col9.substr(4, 2) + "00\n";
        gameshark += "8107EC80 " + col10.substr(0, 2) + col10.substr(2, 2) + "\n";
        gameshark += "8107EC82 " + col10.substr(4, 2) + "00\n";
        gameshark += "8107ECA0 " + col11.substr(0, 2) + col11.substr(2, 2) + "\n";
        gameshark += "8107ECA2 " + col11.substr(4, 2) + "00\n";
        gameshark += "8107EC98 " + col12.substr(0, 2) + col12.substr(2, 2) + "\n";
        gameshark += "8107EC9A " + col12.substr(4, 2) + "00";
        
        //std::cout << gameshark << std::endl;

#ifdef __MINGW32__
        std::ofstream file("addons\\saturn\\assets\\colorcodes\\" + name + ".gs");
#else
        std::ofstream file("addons/saturn/assets/colorcodes/" + name + ".gs");
#endif
        file << gameshark;
    }
}