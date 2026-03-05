#ifndef _gcn64_requests_h__
#define _gcn64_requests_h__

/* Commands */
#define RQ_GCN64_SET_CONFIG_PARAM		0x01
#define RQ_GCN64_GET_CONFIG_PARAM		0x02
#define RQ_GCN64_SUSPEND_POLLING		0x03
#define RQ_GCN64_GET_VERSION			0x04
#define RQ_GCN64_GET_SIGNATURE			0x05
#define RQ_GCN64_GET_CONTROLLER_TYPE	0x06
#define RQ_GCN64_SET_VIBRATION			0x07
#define RQ_GCN64_RAW_SI_COMMAND			0x80
#define RQ_GCN64_BLOCK_IO				0x81
#define RQ_GCN64_JUMP_TO_BOOTLOADER		0xFF

/* Configuration parameters and constants */
#define CFG_PARAM_MODE			0x00

/* Values for mode */
#define CFG_MODE_STANDARD   	0x00
#define CFG_MODE_N64_ONLY		0x01
#define CFG_MODE_GC_ONLY		0x02

#define CFG_PARAM_SERIAL		0x01

#define CFG_PARAM_POLL_INTERVAL0	0x10
#define CFG_PARAM_POLL_INTERVAL1	0x11
#define CFG_PARAM_POLL_INTERVAL2	0x12
#define CFG_PARAM_POLL_INTERVAL3	0x13

#define CFG_PARAM_N64_SQUARE		0x20 // Not implemented
#define CFG_PARAM_GC_MAIN_SQUARE	0x21 // Not implemented
#define CFG_PARAM_GC_CSTICK_SQUARE	0x22 // Not implemented
#define CFG_PARAM_FULL_SLIDERS		0x23
#define CFG_PARAM_INVERT_TRIG		0x24


#endif
