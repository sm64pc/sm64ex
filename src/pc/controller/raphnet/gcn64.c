/*	gc_n64_usb : Gamecube or N64 controller to USB adapter firmware
	Copyright (C) 2007-2017  Raphael Assenat <raph@raphnet.net>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gcn64.h"
#include "gcn64_priv.h"
#include "gcn64lib.h"
#include "requests.h"

#include <hidapi/hidapi.h>

static int dusbr_verbose = 0;

#define IS_VERBOSE()	(dusbr_verbose)

struct supported_adapter {
	uint16_t vid, pid;
	int if_number;
	struct gcn64_adapter_caps caps;
};

static struct supported_adapter supported_adapters[] = {
	/* vid, pid, if_no, { n_raw, bio_support } */

	{ OUR_VENDOR_ID, 0x0017, 1, { 1, 0 } },	// GC/N64 USB v3.0, 3.1.0, 3.1.1
	{ OUR_VENDOR_ID, 0x001D, 1, { 1, 0 } }, // GC/N64 USB v3.2.0 ... v3.3.x
	{ OUR_VENDOR_ID, 0x0020, 1, { 1, 0 } }, // GCN64->USB v3.2.1 (N64 mode)
	{ OUR_VENDOR_ID, 0x0021, 1, { 1, 0 } }, // GCN64->USB v3.2.1 (GC mode)
	{ OUR_VENDOR_ID, 0x0022, 1, { 2, 0 } }, // GCN64->USB v3.3.x (2x GC/N64 mode)
	{ OUR_VENDOR_ID, 0x0030, 1, { 2, 0 } }, // GCN64->USB v3.3.0 (2x N64-only mode)
	{ OUR_VENDOR_ID, 0x0031, 1, { 2, 0 } }, // GCN64->USB v3.3.0 (2x GC-only mode)

	{ OUR_VENDOR_ID, 0x0032, 1, { 1, 1 } }, // GC/N64 USB v3.4.x (GC/N64 mode)
	{ OUR_VENDOR_ID, 0x0033, 1, { 1, 1 } }, // GC/N64 USB v3.4.x (N64 mode)
	{ OUR_VENDOR_ID, 0x0034, 1, { 1, 1 } }, // GC/N64 USB v3.4.x (GC mode)
	{ OUR_VENDOR_ID, 0x0035, 1, { 2, 1 } }, // GC/N64 USB v3.4.x (2x GC/N64 mode)
	{ OUR_VENDOR_ID, 0x0036, 1, { 2, 1 } }, // GC/N64 USB v3.4.x (2x N64-only mode)
	{ OUR_VENDOR_ID, 0x0037, 1, { 2, 1 } }, // GC/N64 USB v3.4.x (2x GC-only mode)

	// GC/N64 USB v3.5.x flavours
	{ OUR_VENDOR_ID, 0x0038, 1, { 1, 1 } }, // (GC/N64 mode)
	{ OUR_VENDOR_ID, 0x0039, 1, { 1, 1 } }, // (N64 mode)
	{ OUR_VENDOR_ID, 0x003A, 1, { 1, 1 } }, // (GC mode)
	{ OUR_VENDOR_ID, 0x003B, 2, { 2, 1 } }, // (2x GC/N64 mode)
	{ OUR_VENDOR_ID, 0x003C, 2, { 2, 1 } }, // (2x N64-only mode)
	{ OUR_VENDOR_ID, 0x003D, 2, { 2, 1 } }, // (2x GC-only mode)

	// GC/N64 USB v3.6.x flavours
	{ OUR_VENDOR_ID, 0x0060, 1, { 1, 1 } }, // (GC/N64 mode)
	{ OUR_VENDOR_ID, 0x0061, 1, { 1, 1 } }, // (N64 mode)
	{ OUR_VENDOR_ID, 0x0063, 2, { 2, 1 } }, // (2x GC/N64 mode)
	{ OUR_VENDOR_ID, 0x0064, 2, { 2, 1 } }, // (2x N64-only mode)
	{ OUR_VENDOR_ID, 0x0067, 1, { 1, 1 } }, // (GC/N64 in GC keyboard mode)

	{ }, // terminator
};

int gcn64_init(int verbose)
{
	dusbr_verbose = verbose;
	hid_init();
	return 0;
}

void gcn64_shutdown(void)
{
	hid_exit();
}

static char isProductIdHandled(unsigned short pid, int interface_number, struct gcn64_adapter_caps *caps)
{
	int i;

	for (i=0; supported_adapters[i].vid; i++) {
		if (pid == supported_adapters[i].pid) {
			if (interface_number == supported_adapters[i].if_number) {
				if (caps) {
					memcpy(caps, &supported_adapters[i].caps, sizeof (struct gcn64_adapter_caps));
				}
				return 1;
			}
		}
	}

	return 0;
}

struct gcn64_list_ctx *gcn64_allocListCtx(void)
{
	struct gcn64_list_ctx *ctx;
	ctx = calloc(1, sizeof(struct gcn64_list_ctx));
	return ctx;
}

void gcn64_freeListCtx(struct gcn64_list_ctx *ctx)
{
	if (ctx) {
		if (ctx->devs) {
			hid_free_enumeration(ctx->devs);
		}
		free(ctx);
	}
}

int gcn64_countDevices(void)
{
	struct gcn64_list_ctx *ctx;
	struct gcn64_info inf;
	int count = 0;

	ctx = gcn64_allocListCtx();
	while (gcn64_listDevices(&inf, ctx)) {
		count++;
	}
	gcn64_freeListCtx(ctx);

	return count;
}

/**
 * \brief List instances of our rgbleds device on the USB busses.
 * \param info Pointer to gcn64_info structure to store data
 * \param dst Destination buffer for device serial number/id.
 * \param dstbuf_size Destination buffer size.
 */
struct gcn64_info *gcn64_listDevices(struct gcn64_info *info, struct gcn64_list_ctx *ctx)
{
	struct gcn64_adapter_caps caps;

	memset(info, 0, sizeof(struct gcn64_info));

	if (!ctx) {
		fprintf(stderr, "gcn64_listDevices: Passed null context\n");
		return NULL;
	}

	if (ctx->devs)
		goto jumpin;

	if (IS_VERBOSE()) {
		printf("Start listing\n");
	}

	ctx->devs = hid_enumerate(OUR_VENDOR_ID, 0x0000);
	if (!ctx->devs) {
		printf("Hid enumerate returned NULL\n");
		return NULL;
	}

	for (ctx->cur_dev = ctx->devs; ctx->cur_dev; ctx->cur_dev = ctx->cur_dev->next)
	{
		if (IS_VERBOSE()) {
			printf("Considering 0x%04x:0x%04x\n", ctx->cur_dev->vendor_id, ctx->cur_dev->product_id);
		}
		if (isProductIdHandled(ctx->cur_dev->product_id, ctx->cur_dev->interface_number, &caps))
		{
				info->usb_vid = ctx->cur_dev->vendor_id;
				info->usb_pid = ctx->cur_dev->product_id;
				wcsncpy(info->str_prodname, ctx->cur_dev->product_string, PRODNAME_MAXCHARS-1);
				wcsncpy(info->str_serial, ctx->cur_dev->serial_number, SERIAL_MAXCHARS-1);
				strncpy(info->str_path, ctx->cur_dev->path, PATH_MAXCHARS-1);
				memcpy(&info->caps, &caps, sizeof(info->caps));
				return info;
		}

		jumpin:
		// prevent 'error: label at end of compound statement'
		continue;
	}

	return NULL;
}

gcn64_hdl_t gcn64_openDevice(struct gcn64_info *dev)
{
	hid_device *hdev;
	gcn64_hdl_t hdl;

	if (!dev)
		return NULL;

	if (IS_VERBOSE()) {
		printf("Opening device path: '%s'\n", dev->str_path);
	}

	hdev = hid_open_path(dev->str_path);
	if (!hdev) {
		return NULL;
	}

	hdl = malloc(sizeof(struct _gcn64_hdl_t));
	if (!hdl) {
		perror("malloc");
		hid_close(hdev);
		return NULL;
	}
	hdl->hdev = hdev;
	hdl->report_size = 63;
	memcpy(&hdl->caps, &dev->caps, sizeof(hdl->caps));

	if (!dev->caps.bio_support) {
		printf("Pre-3.4 version detected. Setting report size to 40 bytes\n");
		hdl->report_size = 40;
	}

	return hdl;
}

gcn64_hdl_t gcn64_openBy(struct gcn64_info *dev, unsigned char flags)
{
	struct gcn64_list_ctx *ctx;
	struct gcn64_info inf;
	gcn64_hdl_t h;

	if (IS_VERBOSE())
		printf("gcn64_openBy, flags=0x%02x\n", flags);

	ctx = gcn64_allocListCtx();
	if (!ctx)
		return NULL;

	while (gcn64_listDevices(&inf, ctx)) {
		if (IS_VERBOSE())
			printf("Considering '%s'\n", inf.str_path);

		if (flags & GCN64_FLG_OPEN_BY_SERIAL) {
			if (wcscmp(inf.str_serial, dev->str_serial))
				continue;
		}

		if (flags & GCN64_FLG_OPEN_BY_PATH) {
			if (strcmp(inf.str_path, dev->str_path))
				continue;
		}

		if (flags & GCN64_FLG_OPEN_BY_VID) {
			if (inf.usb_vid != dev->usb_vid)
				continue;
		}

		if (flags & GCN64_FLG_OPEN_BY_PID) {
			if (inf.usb_pid != dev->usb_pid)
				continue;
		}

		if (IS_VERBOSE())
			printf("Found device. opening...\n");

		h = gcn64_openDevice(&inf);
		gcn64_freeListCtx(ctx);
		return h;
	}

	gcn64_freeListCtx(ctx);
	return NULL;
}

void gcn64_closeDevice(gcn64_hdl_t hdl)
{
	hid_device *hdev = hdl->hdev;

	if (hdev) {
		hid_close(hdev);
	}

	free(hdl);
}

int gcn64_send_cmd(gcn64_hdl_t hdl, const unsigned char *cmd, int cmdlen)
{
	hid_device *hdev = hdl->hdev;
	unsigned char buffer[hdl->report_size+1];
	int n;

	if (cmdlen > (sizeof(buffer)-1)) {
		fprintf(stderr, "Error: Command too long\n");
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));

	buffer[0] = 0x00; // report ID set to 0 (device has only one)
	memcpy(buffer + 1, cmd, cmdlen);

	n = hid_send_feature_report(hdev, buffer, sizeof(buffer));
	if (n < 0) {
		fprintf(stderr, "Could not send feature report (%ls)\n", hid_error(hdev));
		return -1;
	}

	return 0;
}

int gcn64_poll_result(gcn64_hdl_t hdl, unsigned char *cmd, int cmd_maxlen)
{
	hid_device *hdev = hdl->hdev;
	unsigned char buffer[hdl->report_size+1];
	int res_len;
	int n;

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = 0x00; // report ID set to 0 (device has only one)

	n = hid_get_feature_report(hdev, buffer, sizeof(buffer));
	if (n < 0) {
		fprintf(stderr, "Could not send feature report (%ls)\n", hid_error(hdev));
		return -1;
	}
	if (n==0) {
		return 0;
	}
	res_len = n-1;

	if (res_len>0) {
		int copy_len;

		copy_len = res_len;
		if (copy_len > cmd_maxlen) {
			copy_len = cmd_maxlen;
		}
		if (cmd) {
			memcpy(cmd, buffer+1, copy_len);
		}
	}

	return res_len;
}

int gcn64_exchange(gcn64_hdl_t hdl, unsigned char *outcmd, int outlen, unsigned char *result, int result_max)
{
	int n;

	n = gcn64_send_cmd(hdl, outcmd, outlen);
	if (n<0) {
		fprintf(stderr, "Error sending command\n");
		return -1;
	}

	/* Answer to the command comes later. For now, this is polled, but in
	 * the future an interrupt-in transfer could be used. */
	do {
		n = gcn64_poll_result(hdl, result, result_max);
		if (n < 0) {
			fprintf(stderr, "Error\r\n");
			break;
		}
		if (n==0) {
//			printf("."); fflush(stdout);
		}

	} while (n==0);

	return n;
}

