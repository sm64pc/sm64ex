/**
 * This header file provides the interface between modern rendering functionality,
 * written in C++, and the rest of the codebase.
 */

#ifndef RMODERN_H
#define RMODERN_H
#ifdef __cplusplus
    extern "C"
    {
#endif

#include "types.h"

void rmodern_init();

void rm_geo_process_mesh(struct GraphNode* node, Mat4* matrix, s32 animFrame, u16 **animAttributes);

struct GraphNode* rm_load_mesh_from_dl(struct AllocOnlyPool *pool, s32 drawingLayer, void *displayList);

#ifdef __cplusplus
    }
#endif
#endif