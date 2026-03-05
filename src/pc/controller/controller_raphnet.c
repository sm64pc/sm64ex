#ifdef CAPI_RAPHNET

#include <stdarg.h>
#include <stdio.h>

#include "controller_api.h"
#include "raphnet/plugin_back.h"
#include "lib/src/osContInternal.h"
#include "macros.h"

static int l_PluginInit = 0;
static int n_controllers = 0;

static u32 pifRam[16];

static void DebugMessage(int level, const char *message, ...) {
    switch (level) {
        case PB_MSG_ERROR:
            printf("Raphnet: [ERROR] ");
            break;
        case PB_MSG_WARNING:
            printf("Raphnet: [WARNING] ");
            break;
        case PB_MSG_INFO:
            printf("Raphnet: [INFO] ");
            break;
        case PB_MSG_STATUS:
            printf("Raphnet: [STATUS] ");
            break;
        case PB_MSG_VERBOSE:
            printf("Raphnet: [VERBOSE] ");
            break;
        default:
            printf("Raphnet: ");
            break;
    }

    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
}

static void controller_raphnet_init() {
    if (l_PluginInit) {
        return;
    }
    l_PluginInit = 1;

    pb_init(DebugMessage);

    n_controllers = pb_scanControllers();
	if (n_controllers <= 0) {
    	DebugMessage(PB_MSG_ERROR, "No adapters detected\n");
		return;
	}

    pb_romOpen();
}

static void controller_raphnet_read(OSContPad *pad) {
    if (n_controllers <= 0) {
		return;
	}

    u8 *cmdBufPtr;
    OSContPackedRequest request;
    s32 i;
    cmdBufPtr = (u8 *) pifRam;

    for (i = 0; i < 16; i++) {
        pifRam[i] = 0;
    }

    request.padOrEnd = 0xff;
    request.txLen = 1;
    request.rxLen = 4;
    request.command = 1;
    request.data1 = 0xff;
    request.data2 = 0xff;
    request.data3 = 0xff;
    request.data4 = 0xff;
    for (i = 0; i < 4; i++) {
        * (OSContPackedRequest *) cmdBufPtr = request;
        cmdBufPtr += sizeof(OSContPackedRequest);
    }
    *cmdBufPtr = 0xfe;

    pb_readController(0, (u8 *) pifRam + 1);
    pb_readController(-1, NULL);

    OSContPackedRead response = * (OSContPackedRead *) pifRam;
    pad->errnum = (response.rxLen & 0xc0) >> 4;
    if (pad->errnum == 0) {
        pad->button = BE_TO_HOST16(response.button);
        pad->stick_x = response.rawStickX;
        pad->stick_y = response.rawStickY;
    }
}

static u32 controller_raphnet_rawkey() {
    return VK_INVALID;
}

static void controller_raphnet_shutdown() {
    pb_romClosed();
    pb_shutdown();
}

struct ControllerAPI controller_raphnet = {
    VK_INVALID,
    controller_raphnet_init,
    controller_raphnet_read,
    controller_raphnet_rawkey,
    NULL,
    NULL,
    NULL,
    controller_raphnet_shutdown,
};

#endif // CAPI_RAPHNET