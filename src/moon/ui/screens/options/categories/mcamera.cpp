#ifdef BETTERCAMERA

#include "mcamera.h"

#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

MCameraCategory::MCameraCategory() : MoonCategory("TEXT_OPT_CAMERA"){
    this->catOptions.push_back(new MWValue(22, 57,  "TEXT_OPT_CAMON",    {.bvar = &configEnableCamera }, true));
    this->catOptions.push_back(new MWValue(22, 74,  "TEXT_OPT_ANALOGUE", {.bvar = &configCameraAnalog }, true));
    this->catOptions.push_back(new MWValue(22, 91,  "TEXT_OPT_MOUSE",    {.bvar = &configCameraMouse  }, true));
    this->catOptions.push_back(new MWValue(22, 108, "TEXT_OPT_INVERTX",  {.bvar = &configCameraInvertX}, true));
    this->catOptions.push_back(new MWValue(22, 125, "TEXT_OPT_INVERTY",  {.bvar = &configCameraInvertY}, true));
    this->catOptions.push_back(new MWValue(22, 142, "TEXT_OPT_CAMX",     {.ivar = (int*)&configCameraXSens,    .max = 100.0f, .min = .1f, .step = 1.0f}, true));
    this->catOptions.push_back(new MWValue(22, 159, "TEXT_OPT_CAMY",     {.ivar = (int*)&configCameraYSens,    .max = 100.0f, .min = .1f, .step = 1.0f}, true));
    this->catOptions.push_back(new MWValue(22, 176, "TEXT_OPT_CAMC",     {.ivar = (int*)&configCameraAggr,     .max = 100.0f, .min = .0f, .step = 1.0f}, true));
    this->catOptions.push_back(new MWValue(22, 193, "TEXT_OPT_CAMP",     {.ivar = (int*)&configCameraPan,      .max = 100.0f, .min = .0f, .step = 1.0f}, true));
    this->catOptions.push_back(new MWValue(22, 210, "TEXT_OPT_CAMD",     {.ivar = (int*)&configCameraDegrade,  .max = 100.0f, .min = .0f, .step = 1.0f}, true));
}

#endif