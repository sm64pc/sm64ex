#ifndef MoonSaturnEngine
#define MoonSaturnEngine

#include <string>
#include <vector>

namespace MoonInternal {
    void setupSaturnModule(std::string status);
    void freeze_camera();
}

extern bool camera_frozen;
#include "saturn_types.h"

extern bool show_menu_bar;

extern float camera_speed;
extern bool enable_cap_logo;

#endif