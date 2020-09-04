#include "actors/group16.h"

// Why are these scripts compressed unlike the other ones?
// 0x06003754
const GeoLayout chilly_chief_geo[] = {
    GEO_SHADOW(SHADOW_CIRCLE_4_VERTS, 0xC8, 0x46),
    GEO_OPEN_NODE(),
        GEO_SCALE(0, 0x7333),
        GEO_OPEN_NODE(),
            GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
            GEO_OPEN_NODE(),
                GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
                GEO_OPEN_NODE(),
                    GEO_ANIMATED_PART(1, 0, 0, 75, NULL),
                    GEO_OPEN_NODE(),
                        GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
                        GEO_OPEN_NODE(),
                            GEO_ANIMATED_PART(1, 146, 0, 0, NULL),
                            GEO_OPEN_NODE(),
                                GEO_ANIMATED_PART(1, 0, 0, 0, chilly_chief_seg6_dl_06002B30),
                            GEO_CLOSE_NODE(),
                        GEO_CLOSE_NODE(),
                    GEO_CLOSE_NODE(),
                    GEO_ANIMATED_PART(1, 0, 0, -75, NULL),
                    GEO_OPEN_NODE(),
                        GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
                        GEO_OPEN_NODE(),
                            GEO_ANIMATED_PART(1, 146, 0, 0, NULL),
                            GEO_OPEN_NODE(),
                                GEO_ANIMATED_PART(1, 0, 0, 0, chilly_chief_seg6_dl_06002BC8),
                            GEO_CLOSE_NODE(),
                        GEO_CLOSE_NODE(),
                    GEO_CLOSE_NODE(),
                    GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
                    GEO_OPEN_NODE(),
                        GEO_BILLBOARD(),
                        GEO_OPEN_NODE(),
                            GEO_DISPLAY_LIST(LAYER_ALPHA, chilly_chief_seg6_dl_06002D88),
                        GEO_CLOSE_NODE(),
                    GEO_CLOSE_NODE(),
                    GEO_ANIMATED_PART(1, 0, 0, 0, chilly_chief_seg6_dl_06002C60),
                    GEO_ANIMATED_PART(4, 0, 0, 0, chilly_chief_seg6_dl_06003010),
                GEO_CLOSE_NODE(),
            GEO_CLOSE_NODE(),
        GEO_CLOSE_NODE(),
    GEO_CLOSE_NODE(),
    GEO_END(),
};

// 0x06003874
const GeoLayout chilly_chief_big_geo[] = {
    GEO_SHADOW(SHADOW_CIRCLE_4_VERTS, 0xC8, 0xD2),
    GEO_OPEN_NODE(),
        GEO_SCALE(0, 0xE666),
        GEO_OPEN_NODE(),
            GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
            GEO_OPEN_NODE(),
                GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
                GEO_OPEN_NODE(),
                    GEO_ANIMATED_PART(1, 0, 0, 75, NULL),
                    GEO_OPEN_NODE(),
                        GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
                        GEO_OPEN_NODE(),
                            GEO_ANIMATED_PART(1, 146, 0, 0, NULL),
                            GEO_OPEN_NODE(),
                                GEO_ANIMATED_PART(1, 0, 0, 0, chilly_chief_seg6_dl_06002B30),
                            GEO_CLOSE_NODE(),
                        GEO_CLOSE_NODE(),
                    GEO_CLOSE_NODE(),
                    GEO_ANIMATED_PART(1, 0, 0, -75, NULL),
                    GEO_OPEN_NODE(),
                        GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
                        GEO_OPEN_NODE(),
                            GEO_ANIMATED_PART(1, 146, 0, 0, NULL),
                            GEO_OPEN_NODE(),
                                GEO_ANIMATED_PART(1, 0, 0, 0, chilly_chief_seg6_dl_06002BC8),
                            GEO_CLOSE_NODE(),
                        GEO_CLOSE_NODE(),
                    GEO_CLOSE_NODE(),
                    GEO_ANIMATED_PART(1, 0, 0, 0, NULL),
                    GEO_OPEN_NODE(),
                        GEO_BILLBOARD(),
                        GEO_OPEN_NODE(),
                            GEO_DISPLAY_LIST(LAYER_ALPHA, chilly_chief_seg6_dl_06002EF0),
                        GEO_CLOSE_NODE(),
                    GEO_CLOSE_NODE(),
                    GEO_ANIMATED_PART(1, 0, 0, 0, chilly_chief_seg6_dl_06002C60),
                    GEO_ANIMATED_PART(4, 0, 0, 0, chilly_chief_seg6_dl_06003010),
                GEO_CLOSE_NODE(),
            GEO_CLOSE_NODE(),
        GEO_CLOSE_NODE(),
    GEO_CLOSE_NODE(),
    GEO_END(),
};
