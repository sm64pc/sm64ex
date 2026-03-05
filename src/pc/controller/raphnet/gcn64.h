#ifndef _gcn64_h__
#define _gcn64_h__

#include <wchar.h>

#define OUR_VENDOR_ID 	0x289b
#define PRODNAME_MAXCHARS	256
#define SERIAL_MAXCHARS		256
#define PATH_MAXCHARS		256

struct gcn64_adapter_caps {
	int n_raw_channels;
	int bio_support;
};

struct gcn64_info {
	wchar_t str_prodname[PRODNAME_MAXCHARS];
	wchar_t str_serial[SERIAL_MAXCHARS];
	char str_path[PATH_MAXCHARS];
	int usb_vid, usb_pid;
	int access; // True unless direct access to read serial/prodname failed due to permissions.
	struct gcn64_adapter_caps caps;
};

struct gcn64_list_ctx;

typedef struct _gcn64_hdl_t *gcn64_hdl_t; // Cast from hid_device

int gcn64_init(int verbose);
void gcn64_shutdown(void);

struct gcn64_list_ctx *gcn64_allocListCtx(void);
void gcn64_freeListCtx(struct gcn64_list_ctx *ctx);
struct gcn64_info *gcn64_listDevices(struct gcn64_info *info, struct gcn64_list_ctx *ctx);
int gcn64_countDevices(void);

gcn64_hdl_t gcn64_openDevice(struct gcn64_info *dev);

#define GCN64_FLG_OPEN_BY_SERIAL	1	/** Serial must match */
#define GCN64_FLG_OPEN_BY_PATH		2	/** Path must match */
#define GCN64_FLG_OPEN_BY_VID		4	/** USB VID must match */
#define GCN64_FLG_OPEN_BY_PID		8	/** USB PID MUST match */
/**
 * \brief Open a device matching a serial number
 * \param dev The device structure
 * \param flags Flags 
 * \return A handle to the opened device, or NULL if not found
 **/
gcn64_hdl_t gcn64_openBy(struct gcn64_info *dev, unsigned char flags);

void gcn64_closeDevice(gcn64_hdl_t hdl);

int gcn64_send_cmd(gcn64_hdl_t hdl, const unsigned char *cmd, int len);
int gcn64_poll_result(gcn64_hdl_t hdl, unsigned char *cmd, int cmdlen);

int gcn64_exchange(gcn64_hdl_t hdl, unsigned char *outcmd, int outlen, unsigned char *result, int result_max);

#endif // _gcn64_h__

