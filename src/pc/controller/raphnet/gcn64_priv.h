#ifndef _gcn64_priv_h__
#define _gcn64_priv_h__

#include <hidapi/hidapi.h>
#include "gcn64.h"

struct gcn64_list_ctx {
	struct hid_device_info *devs, *cur_dev;
};

typedef struct _gcn64_hdl_t {
	hid_device *hdev;
	int report_size;
	struct gcn64_adapter_caps caps;
} *gcn64_hdl_t;

#endif
