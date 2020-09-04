#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "behavior_table.h"
#include "course_table.h"
#include "src/game/interaction.h"
#include "src/engine/math_util.h"
#include "src/game/save_file.h"
#include "src/menu/file_select.h"
#include "src/pc/fs/fs.h"
#include "PR/os_eeprom.h"

extern u8* gOverrideEeprom;
static u8 eeprom[512] = { 0 };

void network_send_save_file_request(void) {
    assert(networkType == NT_CLIENT);

    gOverrideEeprom = eeprom;

    struct Packet p;
    packet_init(&p, PACKET_SAVE_FILE_REQUEST, true);
    network_send(&p);
}

void network_receive_save_file_request(UNUSED struct Packet* p) {
    assert(networkType == NT_SERVER);
    network_send_save_file();
}

void network_send_save_file(void) {
    assert(networkType == NT_SERVER);

    fs_file_t* fp = fs_open(SAVE_FILENAME);
    if (fp != NULL) {
        fs_read(fp, eeprom, 512);
        fs_close(fp);
    }

    struct Packet p;
    packet_init(&p, PACKET_SAVE_FILE, true);
    packet_write(&p, &gCurrSaveFileNum, sizeof(s16));
    packet_write(&p, eeprom, sizeof(u8) * 512);
    network_send(&p);
}

void network_receive_save_file(struct Packet* p) {
    assert(networkType == NT_CLIENT);

    gOverrideEeprom = eeprom;

    // find all reserved objects
    packet_read(p, &gCurrSaveFileNum, sizeof(s16));
    packet_read(p, eeprom, sizeof(u8) * 512);

    save_file_load_all(TRUE);
    joined_server_as_client(gCurrSaveFileNum);
}
