//*Custom packet examples for sm64ex-coop*\\

/*Packet creation*\
#ifdef COOP
#include "pc/network/network.h"

u8 CUSTOM_PACKET_<name> = 0;

void custom_send_packet_<name>(struct Packet* p, void* params) {
    // write the one u8 parameter
    u8 <name> = (u8)params;
    packet_write(p, &<name>, sizeof(u8));
}

void custom_receive_packet_<name>(struct Packet* p) {
    // read the one u8 parameter
    u8 <name>;
    packet_read(p, &<name>, sizeof(u8));

    switch(<name>) {
        case 1:

    }
}

static void custom_reserve_packets(void) {
    static u8 reservedPackets = FALSE;
    if (reservedPackets) { return; }
    reservedPackets = TRUE;

    // register the custom model packet
    CUSTOM_PACKET_<name> = network_register_custom_packet(custom_send_packet_<name>, custom_receive_packet_<name>);
}
#else
static void custom_reserve_packets(void) { }
#endif


/*Object syncing example*\
        /*CannonAnywhere cheat*/
        if (Cheats.Cann == true && m->controller->buttonDown & L_TRIG
            && m->controller->buttonPressed & U_CBUTTONS) {
            struct Object* cannon = spawn_object_relative(1, 0, 200, 0, o, MODEL_NONE, bhvCannon);
            obj_copy_behavior_params(cannon, o);

            #ifdef COOP 
            cannon->oSyncID = 0;
            cannon->parentObj = cannon;
            network_set_sync_id(cannon);
            struct Object* spawn_objects[] = { cannon };
            u32 models[] = { MODEL_NONE };
            network_send_spawn_objects(spawn_objects, models, 1);
            #endif

            break;
        }
