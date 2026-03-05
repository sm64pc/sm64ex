/* mupen64plus-input-raphnetraw
 *
 * Copyright (C) 2016-2017 Raphael Assenat
 *
 * An input plugin that lets the game under emulation communicate with
 * the controllers directly using the direct controller communication
 * feature of raphnet V3 adapters[1].
 *
 * [1] http://www.raphnet.net/electronique/gcn64_usb_adapter_gen3/index_en.php
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * plugin_back.c : Plugin back code (in contrast to the "front" emulator interface
 *
 * Revision history:
 * 	28 Nov 2016 : Initial version
 * 	 1 Dec 2016 : Switch to block IO api
 *
 */

#include <stdio.h>
#include <string.h>
#include "plugin_back.h"
#include "gcn64.h"
#include "gcn64lib.h"
#include "hexdump.h"

#undef ENABLE_TIMING

#ifdef ENABLE_TIMING
#include <sys/time.h>

#if defined(__MINGW32__) || defined(__MINGW64__)
#error Timing not supported under Windows
#endif
#endif

#undef TIME_RAW_IO
#undef TIME_COMMAND_TO_READ

#undef TRACE_BLOCK_IO
// Filter
#define EXTENSION_RW_ONLY 0

#define MAX_ADAPTERS	4
#define MAX_CHANNELS	4

static void nodebug(int l, const char *m, ...) { }

static pb_debugFunc DebugMessage = nodebug;



#define MAX_OPS	64

struct adapter {
	gcn64_hdl_t handle;
	struct gcn64_info inf;
	struct blockio_op biops[MAX_OPS];
	int n_ops;
};

static int g_n_adapters = 0;
struct adapter g_adapters[MAX_ADAPTERS] = { };

struct rawChannel {
	struct adapter *adapter;
	int chn;
};

/* Multiple adapters are supported, some are single player, others
 * two-player. As they are discovered during scan, their
 * channels (corresponding to physical controller ports) are added
 * to this table. Then, when plugin_front() calls functions in this
 * files with a specified channel, the channel is the index in this
 * table.
 */
static struct rawChannel g_channels[MAX_CHANNELS] = { };
static int g_n_channels = 0;

int pb_init(pb_debugFunc debugFn)
{
	DebugMessage = debugFn;
	gcn64_init(0);
	return 0;
}

static void pb_freeAllAdapters(void)
{
	int i;

	for (i=0; i<g_n_adapters; i++) {
		if (g_adapters[i].handle) {
			/* RomClosed() should have done this, but just
			   in case it is not always called, do this again here. */
			gcn64lib_suspendPolling(g_adapters[i].handle, 0);
			gcn64_closeDevice(g_adapters[i].handle);
		}
	}

	g_n_channels = 0;
	g_n_adapters = 0;
	memset(g_adapters, 0, sizeof(g_adapters));
	memset(g_channels, 0, sizeof(g_channels));
}

int pb_shutdown(void)
{
	pb_freeAllAdapters();
	gcn64_shutdown();

	return 0;
}

/**
 * \return The number of channels available.
 */
int pb_scanControllers(void)
{
	struct gcn64_list_ctx * lctx;
	int i, j;
	struct adapter *adap;

	lctx = gcn64_allocListCtx();
	if (!lctx) {
		DebugMessage(PB_MSG_ERROR, "Could not allocate gcn64 list context\n");
		return 0;
	}

	/* This may be called many times in the plugin's lifetime. For instance, each
	 * time a new game is selected from the PJ64 menu. Freeing previously found
	 * adapters here and creating a new list makes it possible to disconnect/replace
	 * USB adapters without having to restart PJ64. */
	pb_freeAllAdapters();

	/* Pass 1: Fill g_adapters[] with the adapters present on the system. */
	g_n_adapters = 0;
	adap = &g_adapters[g_n_adapters];
	while (gcn64_listDevices(&adap->inf, lctx)) {

		adap->handle = gcn64_openDevice(&adap->inf);
		if (!adap->handle) {
			DebugMessage(PB_MSG_ERROR, "Could not open gcn64 device serial '%ls'. Skipping it.\n", adap->inf.str_serial);
			continue;
		}

		DebugMessage(PB_MSG_INFO, "Found USB device 0x%04x:0x%04x serial '%ls' name '%ls'\n",
						adap->inf.usb_vid, adap->inf.usb_pid, adap->inf.str_serial, adap->inf.str_prodname);
		DebugMessage(PB_MSG_INFO, "Adapter supports %d raw channel(s)\n", adap->inf.caps.n_raw_channels);

		g_n_adapters++;
		if (g_n_adapters >= MAX_ADAPTERS)
			break;

		adap = &g_adapters[g_n_adapters];
	}
	gcn64_freeListCtx(lctx);

	/* Pass 2: Fill the g_channel[] array with the available raw channels.
	 * For instance, if there are adapters A, B and C (where A and C are single-player
	 * and B is dual-player), we get this:
	 *
	 * [0] = Adapter A, raw channel 0
	 * [1] = Adapter B, raw channel 0
	 * [2] = Adapter B, raw channel 1
	 * [3] = Adapter C, raw channel 0
	 *
	 * */
	for (i=0; i<g_n_adapters; i++) {
		struct adapter *adap = &g_adapters[i];

		if (adap->inf.caps.n_raw_channels <= 0)
			continue;

		for (j=0; j<adap->inf.caps.n_raw_channels; j++) {
			if (g_n_channels >= MAX_CHANNELS) {
				return g_n_channels;
			}
			g_channels[g_n_channels].adapter = adap;
			g_channels[g_n_channels].chn = j;
			DebugMessage(PB_MSG_INFO, "Channel %d: Adapter '%ls' raw channel %d\n", g_n_channels, adap->inf.str_serial, j);
			g_n_channels++;
		}
	}

	return g_n_channels;
}

int pb_romOpen(void)
{
	int i;

	for (i=0; i<MAX_ADAPTERS; i++) {
		if (g_adapters[i].handle) {
			gcn64lib_suspendPolling(g_adapters[i].handle, 1);
		}
	}

	return 0;
}

int pb_romClosed(void)
{
	int i;

	for (i=0; i<MAX_ADAPTERS; i++) {
		if (g_adapters[i].handle) {
			gcn64lib_suspendPolling(g_adapters[i].handle, 0);
		}
	}

	return 0;
}

static void timing(int start, const char *label)
{
#ifdef ENABLE_TIMING
	static struct timeval tv_start;
	static int started = 0;
	struct timeval tv_now;

	if (start) {
		gettimeofday(&tv_start, NULL);
		started = 1;
		return;
	}

	if (started) {
		gettimeofday(&tv_now, NULL);
		printf("%s: %ld us\n", label, (tv_now.tv_sec - tv_start.tv_sec) * 1000000 + (tv_now.tv_usec - tv_start.tv_usec) );
		started = 0;
	}
#endif
}

#ifdef _DEBUG
static void debug_raw_commands_in(unsigned char *command, int channel_id)
{
	int tx_len = command[0];
	int rx_len = command[1] & 0x3F;
	int i;

	printf("chn %d: tx[%d] = {", channel_id, tx_len);
	for (i=0; i<tx_len; i++) {
		printf("0x%02x ", command[2+i]);
	}
	printf("}, expecting %d byte%c\n", rx_len, rx_len > 1 ? 's':' ');

}

static void debug_raw_commands_out(unsigned char *command, int channel_id)
{
	int result = command[1];
	int i;

	printf("chn %d: result: 0x%02x : ", channel_id, result);
	if (0 == (command[1] & 0xC0)) {
		for (i=0; i<result; i++) {
			printf("0x%02x ", command[2+i]);
		}
	} else {
		printf("error\n");
	}
	printf("\n");
}

#endif


int pb_controllerCommand(int Control, unsigned char *Command)
{
	// I thought of sending requests to the adapter from here, reading
	// the results later in readController.
	//
	// But unlike what I expected, controllerCommand is NOT always called
	// before readController... Will investigate later.
#ifdef TIME_COMMAND_TO_READ
	timing(1, NULL);
#endif
	return 0;
}

static int pb_performIo(void)
{
	struct adapter *adap;
	struct blockio_op *biops;
	int j, i, res;
	int op_was_extio[MAX_OPS] = { };

	for (j=0; j<g_n_channels; j++)
	{
		adap = &g_adapters[j];
		biops = adap->biops;

		/* Skip adapters that do not have IO operations queued. */
		if (adap->n_ops <= 0)
			continue;

#ifdef TRACE_BLOCK_IO
		for (i=0; i<adap->n_ops; i++) {
			if (EXTENSION_RW_ONLY && (biops[i].tx_data[0] < 0x02))
				continue;
			op_was_extio[i] = 1;
			printf("Before blockIO: op %d, chn: %d, : tx: 0x%02x, rx: 0x%02x, data: ", i, biops[i].chn,
						biops[i].tx_len, biops[i].rx_len);
			printHexBuf(biops[i].tx_data, biops[i].tx_len);
		}
#endif

#ifdef TIME_RAW_IO
		timing(1, NULL);
#endif
		res = gcn64lib_blockIO(adap->handle, biops, adap->n_ops);
#ifdef TIME_RAW_IO
		timing(0, "blockIO");
#endif

		if (res == 0) {
			// biops rx_data pointed into PIFram so data is there. But we need
			// to patch the RX length parameters (the two most significant bits
			// are error bits such as timeout..)
			for (i=0; i<adap->n_ops; i++) {
				// in PIFram, the read length is one byte before the tx data. A rare
				// occasion to use a negative array index ;)
				biops[i].tx_data[-1] = biops[i].rx_len;

#ifdef TRACE_BLOCK_IO
				if (EXTENSION_RW_ONLY && (!op_was_extio[i])) // Read
					continue;

				printf("After blockIO: op %d, chn: %d, : tx: 0x%02x, rx: 0x%02x, data: ", i, biops[i].chn,
						biops[i].tx_len, biops[i].rx_len);
				if (biops[i].rx_len & BIO_RX_LEN_TIMEDOUT) {
					printf("Timeout\n");
				} else if (biops[i].rx_len & BIO_RX_LEN_PARTIAL) {
					printf("Incomplete\n");
				} else {
					printHexBuf(biops[i].rx_data, biops[i].rx_len);
				}
#endif
			}
		} else {
			// For debugging
			//exit(1);
		}

		adap->n_ops = 0;
	}
	return 0;


}

static int pb_commandIsValid(int Control, unsigned char *Command)
{
	if (Control < 0 || Control >= g_n_channels) {
		DebugMessage(PB_MSG_WARNING, "pb_readController called with Control=%d\n", Control);
		return 0;
	}

	if (!Command) {
		DebugMessage(PB_MSG_WARNING, "pb_readController called with NULL Command pointer\n");
		return 0;
	}

	// When a CIC challenge took place in update_pif_write(), the pif ram
	// contains a bunch 0xFF followed by 0x00 at offsets 46 and 47. Offset
	// 48 onwards contains the challenge answer.
	//
	// Then when update_pif_read() is called, the 0xFF bytes are be skipped
	// up to the two 0x00 bytes that increase the channel to 2. Then the
	// challenge answer is (incorrectly) processed as if it were commands
	// for the third controller.
	//
	// This cause issues with the raphnetraw plugin since it modifies pif ram
	// to store the result or command error flags. This corrupts the reponse
	// and leads to challenge failure.
	//
	// As I know of no controller commands above 0x03, the filter below guards
	// against this condition...
	//
	if (Control == 2 && Command[2] > 0x03) {
		DebugMessage(PB_MSG_WARNING, "Invalid controller command\n");
		return 0;
	}

	// When Mario Kart 64 uses a controller pak, such PIF ram content
	// occurs:
	//
	// ff 03 21 02 01 f7 ff ff
	// ff ff ff ff ff ff ff ff
	// ff ff ff ff ff ff ff ff
	// ff ff ff ff ff ff ff ff
	// ff ff ff ff ff ff ff 21
	// fe 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	// 00 00 00 00 00 00 00 00
	//
	// It results in this:
	//  - Transmission of 3 bytes with a 33 bytes return on channel 0,
	//  - Transmission of 33 bytes with 254 bytes in return on channel 1!?
	//
	// Obviously the second transaction is an error. The 0xFE (254) that follows
	// is where processing should actually stop. This happens to be an invalid length detectable
	// by looking at the two most significant bits..
	//
	if (Command[0] == 0xFE && Command[1] == 0x00) {
		DebugMessage(PB_MSG_WARNING, "Ignoring invalid io operation (T: 0x%02x, R: 0x%02x)\n",
			Command[0], Command[1]);
		return 0;
	}

	return 1;
}

int pb_readController(int Control, unsigned char *Command)
{
	struct rawChannel *channel;
	struct adapter *adap;
	struct blockio_op *biops;

	// Called with -1 at the end of PIF ram.
	if (Control == -1) {
		return pb_performIo();
	}

	/* Check for out of bounds Control parameter, for
	 * NULL Command and filter various invalid conditions. */
	if (!pb_commandIsValid(Control, Command)) {
		return 0;
	}

	/* Add the IO operation to the block io list of
	 * the adapter serving this channel. */

	channel = &g_channels[Control];
	adap = channel->adapter;

	if (adap->n_ops >= MAX_OPS) {
		DebugMessage(PB_MSG_ERROR, "Too many io ops\n");
	} else {
		biops = adap->biops;

		biops[adap->n_ops].chn = channel->chn; // Control;
		biops[adap->n_ops].tx_len = Command[0] & BIO_RXTX_MASK;
		biops[adap->n_ops].rx_len = Command[1] & BIO_RXTX_MASK;
		biops[adap->n_ops].tx_data = Command + 2;
		biops[adap->n_ops].rx_data = Command + 2 + biops[adap->n_ops].tx_len;

		if (biops[adap->n_ops].tx_len == 0 || biops[adap->n_ops].rx_len == 0) {
			DebugMessage(PB_MSG_WARNING, "TX or RX was zero\n");
			return 0;
		}

		adap->n_ops++;
	}

	return 0;
}
