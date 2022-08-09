#include "dynos.c.h"
#include "audio/internal.h"
#include "engine/graph_node.h"

bool dynos_sanity_check_geo(s16 graphNodeType) {
    return
        (graphNodeType == GRAPH_NODE_TYPE_ROOT) ||
        (graphNodeType == GRAPH_NODE_TYPE_ORTHO_PROJECTION) ||
        (graphNodeType == GRAPH_NODE_TYPE_PERSPECTIVE) ||
        (graphNodeType == GRAPH_NODE_TYPE_MASTER_LIST) ||
        (graphNodeType == GRAPH_NODE_TYPE_START) ||
        (graphNodeType == GRAPH_NODE_TYPE_LEVEL_OF_DETAIL) ||
        (graphNodeType == GRAPH_NODE_TYPE_SWITCH_CASE) ||
        (graphNodeType == GRAPH_NODE_TYPE_CAMERA) ||
        (graphNodeType == GRAPH_NODE_TYPE_TRANSLATION_ROTATION) ||
        (graphNodeType == GRAPH_NODE_TYPE_TRANSLATION) ||
        (graphNodeType == GRAPH_NODE_TYPE_ROTATION) ||
        (graphNodeType == GRAPH_NODE_TYPE_OBJECT) ||
        (graphNodeType == GRAPH_NODE_TYPE_ANIMATED_PART) ||
        (graphNodeType == GRAPH_NODE_TYPE_BILLBOARD) ||
        (graphNodeType == GRAPH_NODE_TYPE_DISPLAY_LIST) ||
        (graphNodeType == GRAPH_NODE_TYPE_SCALE) ||
        (graphNodeType == GRAPH_NODE_TYPE_SHADOW) ||
        (graphNodeType == GRAPH_NODE_TYPE_OBJECT_PARENT) ||
        (graphNodeType == GRAPH_NODE_TYPE_GENERATED_LIST) ||
        (graphNodeType == GRAPH_NODE_TYPE_BACKGROUND) ||
        (graphNodeType == GRAPH_NODE_TYPE_HELD_OBJ) ||
        (graphNodeType == GRAPH_NODE_TYPE_CULLING_RADIUS);
}

bool dynos_sanity_check_seq(u8 loBits) {
    return
        (loBits < LAYERS_MAX);
}
