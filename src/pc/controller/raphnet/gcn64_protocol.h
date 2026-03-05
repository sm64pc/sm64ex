#ifndef _gcn64_protocol_h__
#define _gcn64_protocol_h__

#define CONTROLLER_IS_ABSENT		0
#define CONTROLLER_IS_N64			1
#define CONTROLLER_IS_GC			2
#define CONTROLLER_IS_GC_KEYBOARD	3
#define CONTROLLER_IS_UNKNOWN		4


/* Return many unknown bits, but two are about the expansion port. */
#define N64_GET_CAPABILITIES		0x00
#define N64_RESET					0xFF
#define N64_CAPS_REPLY_LENGTH		3

#define OFFSET_EXT_REMOVED			22
#define OFFSET_EXT_PRESENT			23

/* Returns button states and axis values */
#define N64_GET_STATUS				0x01
#define N64_GET_STATUS_REPLY_LENGTH	4

/* Read from the expansion bus. */
#define N64_EXPANSION_READ			0x02

/* Write to the expansion bus. */
#define N64_EXPANSION_WRITE			0x03

/* Return information about controller. */
#define GC_GETID					0x00
#define GC_GETID_REPLY_LENGTH		3

/* 3-byte get status command. Returns axis and buttons. Also 
 * controls motor. */
#define GC_GETSTATUS1				0x40
#define GC_GETSTATUS2				0x03
#define GC_GETSTATUS3(rumbling)		((rumbling) ? 0x01 : 0x00)
#define GC_GETSTATUS_REPLY_LENGTH	8

/* 3-byte poll keyboard command.
 * Source: http://hitmen.c02.at/files/yagcd/yagcd/chap9.html#sec9.3.3
 * */
#define GC_POLL_KB1					0x54
#define GC_POLL_KB2					0x00
#define GC_POLL_KB3					0x00

/* Gamecube keycodes are from table 9.3.2:
 * http://hitmen.c02.at/files/yagcd/yagcd/chap9.html#sec9.3.2
 *
 * But the table appears to have been built using a PC keyboard
 * converted (Tototek).
 *
 * I was working with an ASCII keyboard so I made a few adjustments
 * to reflect the /real/ key functions. For instance:
 *
 * LWIN, RWIN and MENU are in fact the Henkan, muhenkan and katakana/hiragana keys.
 * They are equivalemnt to the HID keys International 4, International 5 and International 2.
 * The - key (code 0x3F) is also HID international 1.
 * The PrntScrn key (code 0x36) is in fact the Yen key (International 3).
 * The bracket/brace [{ and }] keys are at different places on this Japanese keyboard,
 * but following standard PC practice, some keycodes are named after the US usage. Hence,
 * the aforementioned keys are the 2 keys right of P, even if they are not labeled.
 */
#define GC_KEY_RESERVED			0x00 // No key

#define GC_KEY_HOME				0x06
#define GC_KEY_END				0x07
#define GC_KEY_PGUP				0x08
#define GC_KEY_PGDN				0x09
#define GC_KEY_SCROLL_LOCK		0x0A

#define GC_KEY_A				0x10
#define GC_KEY_B				0x11
#define GC_KEY_C				0x12
#define GC_KEY_D				0x13
#define GC_KEY_E				0x14
#define GC_KEY_F				0x15
#define GC_KEY_G				0x16
#define GC_KEY_H				0x17
#define GC_KEY_I				0x18
#define GC_KEY_J				0x19
#define GC_KEY_K				0x1A
#define GC_KEY_L				0x1B
#define GC_KEY_M				0x1C
#define GC_KEY_N				0x1D
#define GC_KEY_O				0x1E
#define GC_KEY_P				0x1F
#define GC_KEY_Q				0x20
#define GC_KEY_R				0x21
#define GC_KEY_S				0x22
#define GC_KEY_T				0x23
#define GC_KEY_U				0x24
#define GC_KEY_V				0x25
#define GC_KEY_W				0x26
#define GC_KEY_X				0x27
#define GC_KEY_Y				0x28
#define GC_KEY_Z				0x29
#define GC_KEY_1				0x2A
#define GC_KEY_2				0x2B
#define GC_KEY_3				0x2C
#define GC_KEY_4				0x2D
#define GC_KEY_5				0x2E
#define GC_KEY_6				0x2F
#define GC_KEY_7				0x30
#define GC_KEY_8				0x31
#define GC_KEY_9				0x32
#define GC_KEY_0				0x33
#define GC_KEY_DASH_UNDERSCORE	0x34 // Key next to 0
#define GC_KEY_PLUS_EQUAL		0x35
#define GC_KEY_YEN				0x36 // Yen on ascii keyboard. HID International 3.
#define GC_KEY_OPEN_BRKT_BRACE	0x37 // 1st key right of P
#define GC_KEY_CLOSE_BRKT_BRACE	0x38 // 2nd key right of P
#define GC_KEY_SEMI_COLON_COLON	0x39 // ;:
#define GC_KEY_QUOTES			0x3A // '"

// The 3rd key after 'L'. HID Keyboard non-us # and ~
#define GC_KEY_BRACKET_MU		0x3B
#define GC_KEY_COMMA_ST			0x3C // ,<
#define GC_KEY_PERIOD_GT		0x3D // .>
#define GC_KEY_SLASH_QUESTION	0x3E // /?

// (The extra key before right-shift on japanese keyboards. 
// HID code International 1 [HID usage tables Footnote 15-20]).
#define GC_KEY_INTERNATIONAL1	0x3F 
#define GC_KEY_F1				0x40
#define GC_KEY_F2				0x41
#define GC_KEY_F3				0x42
#define GC_KEY_F4				0x43
#define GC_KEY_F5				0x44
#define GC_KEY_F6				0x45
#define GC_KEY_F7				0x46
#define GC_KEY_F8				0x47
#define GC_KEY_F9				0x48
#define GC_KEY_F10				0x49
#define GC_KEY_F11				0x4A
#define GC_KEY_F12				0x4B
#define GC_KEY_ESC				0x4C
#define GC_KEY_INSERT			0x4D
#define GC_KEY_DELETE			0x4E

// (Hankaku/zenkaku/kanji button). Also known as `~
#define GC_KEY_HANKAKU			0x4F 
#define GC_KEY_BACKSPACE		0x50
#define GC_KEY_TAB				0x51

#define GC_KEY_CAPS_LOCK		0x53
#define GC_KEY_LEFT_SHIFT		0x54
#define GC_KEY_RIGHT_SHIFT		0x55
#define GC_KEY_LEFT_CTRL		0x56
#define GC_KEY_LEFT_ALT			0x57
#define GC_KEY_MUHENKAN			0x58 // HID international 5
#define GC_KEY_SPACE			0x59
#define GC_KEY_HENKAN			0x5A // HID international 4
#define GC_KEY_KANA				0x5B // HID international 2
#define GC_KEY_LEFT				0x5C
#define GC_KEY_DOWN				0x5D
#define GC_KEY_UP				0x5E
#define GC_KEY_RIGHT			0x5F

#define GC_KEY_ENTER			0x61

void gcn64protocol_hwinit(void);
int gcn64_detectController(void);
unsigned char gcn64_transaction(const unsigned char *tx, int tx_len, unsigned char *rx, unsigned char rx_max);

#endif // _gcn64_protocol_h__
