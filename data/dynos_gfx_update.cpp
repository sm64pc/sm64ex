#include "dynos.cpp.h"
extern "C" {
#include "object_fields.h"
#include "game/level_update.h"
#include "game/object_list_processor.h"
}

//
// Free data
// Must be unloaded the next frame to prevent a crash
//

STATIC_STORAGE(Array<GfxData *>, GfxDataFreeList);
#define sGfxDataFreeList __GfxDataFreeList()

STATIC_STORAGE(Array<GraphNode *>, GraphNodeFreeList);
#define sGraphNodeFreeList __GraphNodeFreeList()

static void DynOS_Gfx_FreeUnloaded() {
    for (auto &_GfxData : sGfxDataFreeList) DynOS_Gfx_Free(_GfxData);
    for (auto &_GraphNode : sGraphNodeFreeList) Delete(_GraphNode);
    sGfxDataFreeList.Clear();
    sGraphNodeFreeList.Clear();
}

//
// Update animations
//

// Retrieve the current Mario's animation index
static s32 RetrieveCurrentMarioAnimationIndex() {
    struct MarioAnimDmaRelatedThing *_AnimDmaTable = gMarioState->animation->animDmaTable;
    for (s32 i = 0; i != (s32) _AnimDmaTable->count; ++i) {
        void *_AnimAddr = _AnimDmaTable->srcAddr + _AnimDmaTable->anim[i].offset;
        if (_AnimAddr == gMarioState->animation->currentAnimAddr) {
            return i;
        }
    }
    return -1;
}

// Retrieve the current animation index
// As we don't know the length of the table, let's hope that we'll always find the animation...
static s32 RetrieveCurrentAnimationIndex(struct Object *aObject) {
    if (!aObject->oAnimations || !aObject->header.gfx.unk38.curAnim) {
        return -1;
    }
    for (s32 i = 0; aObject->oAnimations[i] != NULL; ++i) {
        if (aObject->oAnimations[i] == aObject->header.gfx.unk38.curAnim) {
            return i;
        }
    }
    return -1;
}

// Must be called twice, before and after geo_set_animation_globals
void DynOS_Gfx_SwapAnimations(void *aPtr) {
    static Animation *pDefaultAnimation = NULL;
    static Animation  sGfxDataAnimation;

    // Does the object has a model?
    struct Object *_Object = (struct Object *) aPtr;
    if (!_Object->header.gfx.sharedChild) {
        return;
    }

    // Swap the current animation with the one from the Gfx data
    if (!pDefaultAnimation) {
        pDefaultAnimation = _Object->header.gfx.unk38.curAnim;

        // Actor index
        s32 _ActorIndex = DynOS_Geo_GetActorIndex(_Object->header.gfx.sharedChild->georef);
        if (_ActorIndex == -1) {
            return;
        }

        // Gfx data
        GfxData *_GfxData = DynOS_Gfx_GetActorList()[_ActorIndex].mGfxData;
        if (!_GfxData) {
            return;
        }

        // Animation table
        if (_GfxData->mAnimationTable.Empty()) {
            return;
        }

        // Animation index
        s32 _AnimIndex = (_Object == gMarioObject ? RetrieveCurrentMarioAnimationIndex() : RetrieveCurrentAnimationIndex(_Object));
        if (_AnimIndex == -1) {
            return;
        }

        // Animation data
        const AnimData *_AnimData = (const AnimData *) _GfxData->mAnimationTable[_AnimIndex].second;
        if (_AnimData) {
            sGfxDataAnimation.flags = _AnimData->mFlags;
            sGfxDataAnimation.unk02 = _AnimData->mUnk02;
            sGfxDataAnimation.unk04 = _AnimData->mUnk04;
            sGfxDataAnimation.unk06 = _AnimData->mUnk06;
            sGfxDataAnimation.unk08 = _AnimData->mUnk08;
            sGfxDataAnimation.unk0A = _AnimData->mUnk0A.second;
            sGfxDataAnimation.values = _AnimData->mValues.second.begin();
            sGfxDataAnimation.index = _AnimData->mIndex.second.begin();
            sGfxDataAnimation.length = _AnimData->mLength;
            _Object->header.gfx.unk38.curAnim = &sGfxDataAnimation;
        }

    // Restore the default animation
    } else {
        _Object->header.gfx.unk38.curAnim = pDefaultAnimation;
        pDefaultAnimation = NULL;
    }
}

//
// Update models
//

static void DynOS_Gfx_UpdateModelData(struct Object *aObject, s32 aActorIndex) {
    ActorGfx *_ActorGfx = &DynOS_Gfx_GetActorList()[aActorIndex];
    const Array<PackData *> &pDynosPacks = DynOS_Gfx_GetPacks();
    for (s32 i = 0; i != pDynosPacks.Count(); ++i) {

        // Pack
        bool _Enabled = DynOS_Opt_GetValue(String("dynos_pack_%d", i));

        // If enabled and no pack is selected
        // load the pack's model and replace the default actor's model
        if (_Enabled && _ActorGfx->mPackIndex == -1) {

            // Load Gfx data from binary
            SysPath _Filename = fstring("%s/%s.bin", pDynosPacks[i]->mPath.begin(), DynOS_Geo_GetActorName(aActorIndex));
            GfxData *_GfxData = DynOS_Gfx_LoadFromBinary(_Filename);
            if (_GfxData == NULL) {
                continue;
            }

            // Mark previous model data as unload
            if (_ActorGfx->mGfxData) sGfxDataFreeList.Add(_ActorGfx->mGfxData);
            if (_ActorGfx->mGraphNode) sGraphNodeFreeList.Add(_ActorGfx->mGraphNode);

            // Load graph node and animations
            _ActorGfx->mPackIndex = i;
            _ActorGfx->mGfxData   = _GfxData;
            _ActorGfx->mGraphNode = (GraphNode *) DynOS_Geo_GetGraphNode((*(_GfxData->mGeoLayouts.end() - 1))->mData, false);
            _ActorGfx->mGraphNode->georef = DynOS_Geo_GetActorLayout(aActorIndex);
            break;
        }

        // If disabled and this pack is the one selected
        // unload the pack's model and replace the actor's model by the default one
        else if (!_Enabled && _ActorGfx->mPackIndex == i) {

            // Mark previous model data as unload
            if (_ActorGfx->mGfxData) sGfxDataFreeList.Add(_ActorGfx->mGfxData);
            if (_ActorGfx->mGraphNode) sGraphNodeFreeList.Add(_ActorGfx->mGraphNode);

            // Default
            _ActorGfx->mPackIndex = -1;
            _ActorGfx->mGfxData   = NULL;
            _ActorGfx->mGraphNode = (GraphNode *) DynOS_Geo_GetGraphNode(DynOS_Geo_GetActorLayout(aActorIndex), false);
        }
    }

    // Update object
    aObject->header.gfx.sharedChild = _ActorGfx->mGraphNode;
}

void DynOS_Gfx_Update() {

    // Don't update until the object lists are loaded
    if (!gObjectLists) {
        return;
    }

    // Free unloaded things
    DynOS_Gfx_FreeUnloaded();

    // Update per object
    for (s32 _List = 0; _List != NUM_OBJ_LISTS; ++_List) {
        struct Object *_Head = (struct Object *) &gObjectLists[_List];
        for (struct Object *_Object = (struct Object *) _Head->header.next; _Object != _Head; _Object = (struct Object *) _Object->header.next) {

            // Does the object has a model?
            if (!_Object->header.gfx.sharedChild) {
                continue;
            }

            // Actor index
            s32 _ActorIndex = DynOS_Geo_GetActorIndex(_Object->header.gfx.sharedChild->georef);
            if (_ActorIndex == -1) {
                continue;
            }

            // Replace the object's model and animations
            DynOS_Gfx_UpdateModelData(_Object, _ActorIndex);
        }
    }
}
