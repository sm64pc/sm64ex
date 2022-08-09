#include "dynos.cpp.h"
extern "C" {
#include "pc/gfx/gfx_rendering_api.h"
}

//
// Conversion
//

#define SCALE_5_8(VAL_) (((VAL_) * 0xFF) / 0x1F)
#define SCALE_8_5(VAL_) ((((VAL_) + 4) * 0x1F) / 0xFF)
#define SCALE_4_8(VAL_) ((VAL_) * 0x11)
#define SCALE_8_4(VAL_) ((VAL_) / 0x11)
#define SCALE_3_8(VAL_) ((VAL_) * 0x24)
#define SCALE_8_3(VAL_) ((VAL_) / 0x24)

static u8 *RGBA16_RGBA32(const u8 *aData, u64 aLength) {
    u8 *_Buffer = New<u8>(aLength * 2);
    u8 *pBuffer = _Buffer;
    for (u64 i = 0; i != aLength; i += 2) {
        u16 _Col  = (aData[i + 0] << 8) | aData[i + 1];
        u8  _Red  = (_Col >> 11) & 0x1F;
        u8  _Grn  = (_Col >>  6) & 0x1F;
        u8  _Blu  = (_Col >>  1) & 0x1F;
        u8  _Alp  = (_Col >>  0) & 0x01;
        *(pBuffer++) = (SCALE_5_8(_Red));
        *(pBuffer++) = (SCALE_5_8(_Grn));
        *(pBuffer++) = (SCALE_5_8(_Blu));
        *(pBuffer++) = (0xFF  *  (_Alp));
    }
    return _Buffer;
}

static u8 *RGBA32_RGBA32(const u8 *aData, u64 aLength) {
    u8 *_Buffer = New<u8>(aLength * 1);
    memcpy(_Buffer, aData, aLength);
    return _Buffer;
}

static u8 *IA4_RGBA32(const u8 *aData, u64 aLength) {
    u8 *_Buffer = New<u8>(aLength * 8);
    u8 *pBuffer = _Buffer;
    for (u64 i = 0; i != aLength; ++i) {
        u8 _Half0 = (aData[i] >> 4) & 0xF;
        *(pBuffer++) = (SCALE_3_8(_Half0 >> 1));
        *(pBuffer++) = (SCALE_3_8(_Half0 >> 1));
        *(pBuffer++) = (SCALE_3_8(_Half0 >> 1));
        *(pBuffer++) = (0xFF  *  (_Half0 &  1));

        u8 _Half1 = (aData[i] >> 0) & 0xF;
        *(pBuffer++) = (SCALE_3_8(_Half1 >> 1));
        *(pBuffer++) = (SCALE_3_8(_Half1 >> 1));
        *(pBuffer++) = (SCALE_3_8(_Half1 >> 1));
        *(pBuffer++) = (0xFF  *  (_Half1 &  1));
    }
    return _Buffer;
}

static u8 *IA8_RGBA32(const u8 *aData, u64 aLength) {
    u8 *_Buffer = New<u8>(aLength * 4);
    u8 *pBuffer = _Buffer;
    for (u64 i = 0; i != aLength; ++i) {
        u8 _Col   = (aData[i] >> 4) & 0xF;
        u8 _Alp   = (aData[i] >> 0) & 0xF;
        *(pBuffer++) = (SCALE_4_8(_Col));
        *(pBuffer++) = (SCALE_4_8(_Col));
        *(pBuffer++) = (SCALE_4_8(_Col));
        *(pBuffer++) = (SCALE_4_8(_Alp));
    }
    return _Buffer;
}

static u8 *IA16_RGBA32(const u8 *aData, u64 aLength) {
    u8 *_Buffer = New<u8>(aLength * 2);
    u8 *pBuffer = _Buffer;
    for (u64 i = 0; i != aLength; i += 2) {
        u8 _Col   = aData[i + 0];
        u8 _Alp   = aData[i + 1];
        *(pBuffer++) = (_Col);
        *(pBuffer++) = (_Col);
        *(pBuffer++) = (_Col);
        *(pBuffer++) = (_Alp);
    }
    return _Buffer;
}

static u8 *CI4_RGBA32(const u8 *aData, u64 aLength, const u8 *aPalette) {
    u8 *_Buffer = New<u8>(aLength * 8);
    u8 *pBuffer = _Buffer;
    for (u64 i = 0; i != aLength; ++i) {
        u8  _Idx0 = (aData[i] >> 4) & 0xF;
        u16 _Col0 = (aPalette[_Idx0 * 2 + 0] << 8) | aPalette[_Idx0 * 2 + 1];
        u8  _Red0 = (_Col0 >> 11) & 0x1F;
        u8  _Grn0 = (_Col0 >>  6) & 0x1F;
        u8  _Blu0 = (_Col0 >>  1) & 0x1F;
        u8  _Alp0 = (_Col0 >>  0) & 0x01;
        *(pBuffer++) = (SCALE_5_8(_Red0));
        *(pBuffer++) = (SCALE_5_8(_Grn0));
        *(pBuffer++) = (SCALE_5_8(_Blu0));
        *(pBuffer++) = (0xFF  *  (_Alp0));

        u8  _Idx1 = (aData[i] >> 0) & 0xF;
        u16 _Col1 = (aPalette[_Idx1 * 2 + 0] << 8) | aPalette[_Idx1 * 2 + 1];
        u8  _Red1 = (_Col1 >> 11) & 0x1F;
        u8  _Grn1 = (_Col1 >>  6) & 0x1F;
        u8  _Blu1 = (_Col1 >>  1) & 0x1F;
        u8  _Alp1 = (_Col1 >>  0) & 0x01;
        *(pBuffer++) = (SCALE_5_8(_Red1));
        *(pBuffer++) = (SCALE_5_8(_Grn1));
        *(pBuffer++) = (SCALE_5_8(_Blu1));
        *(pBuffer++) = (0xFF  *  (_Alp1));
    }
    return _Buffer;
}

static u8 *CI8_RGBA32(const u8 *aData, u64 aLength, const u8 *aPalette) {
    u8 *_Buffer = New<u8>(aLength * 4);
    u8 *pBuffer = _Buffer;
    for (u64 i = 0; i != aLength; ++i) {
        u8  _Idx  = aData[i];
        u16 _Col  = (aPalette[_Idx * 2 + 0] << 8) | aPalette[_Idx * 2 + 1];
        u8  _Red  = (_Col >> 11) & 0x1F;
        u8  _Grn  = (_Col >>  6) & 0x1F;
        u8  _Blu  = (_Col >>  1) & 0x1F;
        u8  _Alp  = (_Col >>  0) & 0x01;
        *(pBuffer++) = (SCALE_5_8(_Red));
        *(pBuffer++) = (SCALE_5_8(_Grn));
        *(pBuffer++) = (SCALE_5_8(_Blu));
        *(pBuffer++) = (0xFF  *  (_Alp));
    }
    return _Buffer;
}

static u8 *I4_RGBA32(const u8 *aData, u64 aLength) {
    u8 *_Buffer = New<u8>(aLength * 8);
    u8 *pBuffer = _Buffer;
    for (u64 i = 0; i != aLength; ++i) {
        u8 _Half0 = (aData[i] >> 4) & 0xF;
        *(pBuffer++) = (SCALE_4_8(_Half0));
        *(pBuffer++) = (SCALE_4_8(_Half0));
        *(pBuffer++) = (SCALE_4_8(_Half0));
        *(pBuffer++) = (255);

        u8 _Half1 = (aData[i] >> 0) & 0xF;
        *(pBuffer++) = (SCALE_4_8(_Half1));
        *(pBuffer++) = (SCALE_4_8(_Half1));
        *(pBuffer++) = (SCALE_4_8(_Half1));
        *(pBuffer++) = (255);
    }
    return _Buffer;
}

static u8 *I8_RGBA32(const u8 *aData, u64 aLength) {
    u8 *_Buffer = New<u8>(aLength * 4);
    u8 *pBuffer = _Buffer;
    for (u64 i = 0; i != aLength; ++i) {
        *(pBuffer++) = (aData[i]);
        *(pBuffer++) = (aData[i]);
        *(pBuffer++) = (aData[i]);
        *(pBuffer++) = (255);
    }
    return _Buffer;
}

u8 *DynOS_Gfx_TextureConvertToRGBA32(const u8 *aData, u64 aLength, s32 aFormat, s32 aSize, const u8 *aPalette) {
    switch   ((aFormat       << 8) | aSize       ) {
        case ((G_IM_FMT_RGBA << 8) | G_IM_SIZ_16b): return RGBA16_RGBA32(aData, aLength);
        case ((G_IM_FMT_RGBA << 8) | G_IM_SIZ_32b): return RGBA32_RGBA32(aData, aLength);
        case ((G_IM_FMT_IA   << 8) | G_IM_SIZ_4b ): return IA4_RGBA32   (aData, aLength);
        case ((G_IM_FMT_IA   << 8) | G_IM_SIZ_8b ): return IA8_RGBA32   (aData, aLength);
        case ((G_IM_FMT_IA   << 8) | G_IM_SIZ_16b): return IA16_RGBA32  (aData, aLength);
        case ((G_IM_FMT_CI   << 8) | G_IM_SIZ_4b ): return CI4_RGBA32   (aData, aLength, aPalette);
        case ((G_IM_FMT_CI   << 8) | G_IM_SIZ_8b ): return CI8_RGBA32   (aData, aLength, aPalette);
        case ((G_IM_FMT_I    << 8) | G_IM_SIZ_4b ): return I4_RGBA32    (aData, aLength);
        case ((G_IM_FMT_I    << 8) | G_IM_SIZ_8b ): return I8_RGBA32    (aData, aLength);
    }
    return NULL;
}

//
// Upload
//

typedef struct GfxRenderingAPI GRAPI;
static void DynOS_Gfx_UploadTexture(DataNode<TexData> *aNode, GRAPI *aGfxRApi, s32 aTile, s32 aTexId) {
    aGfxRApi->select_texture(aTile, aTexId);
    aGfxRApi->upload_texture(aNode->mData->mRawData.begin(), aNode->mData->mRawWidth, aNode->mData->mRawHeight);
    aNode->mData->mUploaded = true;
}

//
// Cache
//

struct THN {
    struct THN *mNext;
    const void *mAddr; // Contains the pointer to the DataNode<TexData> struct, NOT the actual texture data
    u8 mFmt, mSiz;
    s32 mTexId;
    u8 mCms, mCmt;
    bool mLInf;
};

static bool DynOS_Gfx_CacheTexture(THN **aOutput, DataNode<TexData> *aNode, s32 aTile, GRAPI *aGfxRApi, THN **aHashMap, THN *aPool, u32 *aPoolPos, u32 aPoolSize) {

    // Find texture in cache
    uintptr_t _Hash = ((uintptr_t) aNode) & ((aPoolSize * 2) - 1);
    THN **_Node = &(aHashMap[_Hash]);
    while ((*_Node) != NULL && ((*_Node) - aPool) < (*aPoolPos)) {
        if ((*_Node)->mAddr == (const void *) aNode) {
            aGfxRApi->select_texture(aTile, (*_Node)->mTexId);
            if (!aNode->mData->mUploaded) {
                DynOS_Gfx_UploadTexture(aNode, aGfxRApi, aTile, (*_Node)->mTexId);
            }
            (*aOutput) = (*_Node);
            return true;
        }
        _Node = &(*_Node)->mNext;
    }

    // If cache is full, clear cache
    if ((*aPoolPos) == aPoolSize) {
        (*aPoolPos) = 0;
        _Node = &aHashMap[_Hash];
    }

    // Add new texture to cache
    (*_Node) = &aPool[(*aPoolPos)++];
    if (!(*_Node)->mAddr) {
        (*_Node)->mTexId = aGfxRApi->new_texture();
    }
    aGfxRApi->select_texture(aTile, (*_Node)->mTexId);
    aGfxRApi->set_sampler_parameters(aTile, false, 0, 0);
    (*_Node)->mCms  = 0;
    (*_Node)->mCmt  = 0;
    (*_Node)->mLInf = false;
    (*_Node)->mNext = NULL;
    (*_Node)->mAddr = aNode;
    (*_Node)->mFmt  = G_IM_FMT_RGBA;
    (*_Node)->mSiz  = G_IM_SIZ_32b;
    (*aOutput)      = (*_Node);
    return false;
}

//
// Import
//

static DataNode<TexData> *DynOS_Gfx_RetrieveNode(void *aPtr) {
    Array<ActorGfx> &pActorGfxList = DynOS_Gfx_GetActorList();
    for (auto& _ActorGfx : pActorGfxList) {
        if (_ActorGfx.mGfxData) {
            for (auto &_Node : _ActorGfx.mGfxData->mTextures) {
                if ((void*) _Node == aPtr) {
                    return _Node;
                }
            }
        }
    }
    return NULL;
}

static bool DynOS_Gfx_ImportTexture_Typed(THN **aOutput, void *aPtr, s32 aTile, GRAPI *aGfxRApi, THN **aHashMap, THN *aPool, u32 *aPoolPos, u32 aPoolSize) {
    DataNode<TexData> *_Node = DynOS_Gfx_RetrieveNode(aPtr);
    if (_Node) {
        if (!DynOS_Gfx_CacheTexture(aOutput, _Node, aTile, aGfxRApi, aHashMap, aPool, aPoolPos, aPoolSize)) {
            DynOS_Gfx_UploadTexture(_Node, aGfxRApi, aTile, (*aOutput)->mTexId);
        }
        return true;
    }
    return false;
}

bool DynOS_Gfx_ImportTexture(void **aOutput, void *aPtr, s32 aTile, void *aGfxRApi, void **aHashMap, void *aPool, u32 *aPoolPos, u32 aPoolSize) {
    return DynOS_Gfx_ImportTexture_Typed(
        (THN **)  aOutput,
        (void *)  aPtr,
        (s32)     aTile,
        (GRAPI *) aGfxRApi,
        (THN **)  aHashMap,
        (THN *)   aPool,
        (u32 *)   aPoolPos,
        (u32)     aPoolSize
    );
}
