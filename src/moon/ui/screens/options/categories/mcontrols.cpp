#include "mcontrols.h"

#include "moon/texts/moon-loader.h"
#include "moon/ui/widgets/mw-value.h"

using namespace std;

extern "C" {
#include "pc/configfile.h"
}

MControllerCategory::MControllerCategory() : MoonCategory("TEXT_OPT_AUDIO"){

}