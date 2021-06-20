#include "mcontrols.h"

#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

MControllerCategory::MControllerCategory() : MoonCategory("TEXT_OPT_CONTROLS"){
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_A",       { .max = 3, .bindKeys = configKeyA,          .keyIcon = "a-alt-btn.rgba16"     }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_B",       { .max = 3, .bindKeys = configKeyB,          .keyIcon = "b-alt-btn.rgba16"     }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_START",   { .max = 3, .bindKeys = configKeyStart,      .keyIcon = "start-btn.rgba16"     }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_L",       { .max = 3, .bindKeys = configKeyL,          .keyIcon = "l-btn.rgba16"         }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_R",       { .max = 3, .bindKeys = configKeyR,          .keyIcon = "r-btn.rgba16"         }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_Z",       { .max = 3, .bindKeys = configKeyZ,          .keyIcon = "z-btn.rgba16"         }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_C_UP",    { .max = 3, .bindKeys = configKeyCUp,        .keyIcon = "c-up.rgba16"          }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_C_DOWN",  { .max = 3, .bindKeys = configKeyCDown,      .keyIcon = "c-down.rgba16"        }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_C_LEFT",  { .max = 3, .bindKeys = configKeyCLeft,      .keyIcon = "c-left.rgba16"        }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_C_RIGHT", { .max = 3, .bindKeys = configKeyCRight,     .keyIcon = "c-right.rgba16"       }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_UP",      { .max = 3, .bindKeys = configKeyStickUp,    .keyIcon = "stick-up.rgba16"      }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_DOWN",    { .max = 3, .bindKeys = configKeyStickDown,  .keyIcon = "stick-down.rgba16"    }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_LEFT",    { .max = 3, .bindKeys = configKeyStickLeft,  .keyIcon = "stick-left.rgba16"    }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_BIND_RIGHT",   { .max = 3, .bindKeys = configKeyStickRight, .keyIcon = "stick-right.rgba16"   }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_OPT_DEADZONE", { .ivar = (int*)&configStickDeadzone,  .max = 100.0f, .min = .0f, .step = 1.0f }, true));
    this->catOptions.push_back(new MWValue(22, 0, "TEXT_OPT_RUMBLE",   { .ivar = (int*)&configRumbleStrength, .max = 127.0f, .min = .0f, .step = 1.0f }, true));
}