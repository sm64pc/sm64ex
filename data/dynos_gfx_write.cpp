#include "dynos.cpp.h"

//
// Pointers
//

typedef Pair<String, u32> PointerData;
static PointerData GetDataFromPointer(const void* aPtr, GfxData* aGfxData) {

    // Lights
    for (auto& _Node : aGfxData->mLights) {
        if (&_Node->mData->l[0] == aPtr) { // Light *, not Lights1 *
            return { _Node->mName, 1 };
        }
        if (&_Node->mData->a == aPtr) { // Ambient *, not Lights1 *
            return { _Node->mName, 2 };
        }
    }

    // Textures
    for (auto& _Node : aGfxData->mTextures) {
        if (_Node == aPtr) {
            return { _Node->mName, 0 };
        }
    }

    // Display lists
    for (auto& _Node : aGfxData->mDisplayLists) {
        if (_Node == aPtr) {
            return { _Node->mName, 0 };
        }
    }

    // Geo layouts
    for (auto& _Node : aGfxData->mGeoLayouts) {
        if (_Node->mData == aPtr) {
            return { _Node->mName, 0 };
        }
    }

    // Vertices
    String _VtxArrayName = "";
    uintptr_t _VtxArrayStart = 0;
    for (auto& _Node : aGfxData->mVertices) {
        if (_Node->mData == aPtr) {
            return { _Node->mName, 0 };
        }
        if ((uintptr_t)_Node->mData <= (uintptr_t)aPtr &&
            (uintptr_t)_Node->mData >= _VtxArrayStart) {
            _VtxArrayName = _Node->mName;
            _VtxArrayStart = (uintptr_t)_Node->mData;
        }
    }
    return { _VtxArrayName, (u32)((const Vtx*)aPtr - (const Vtx*)_VtxArrayStart) };
}

static void WritePointer(FILE* aFile, const void* aPtr, GfxData* aGfxData) {

    // NULL
    if (!aPtr) {
        WriteBytes<u32>(aFile, 0);
        return;
    }

    // Geo function
    s32 _GeoFunctionIndex = DynOS_Geo_GetFunctionIndex(aPtr);
    if (_GeoFunctionIndex != -1) {
        WriteBytes<u32>(aFile, FUNCTION_CODE);
        WriteBytes<s32>(aFile, _GeoFunctionIndex);
        return;
    }

    // Pointer
    PointerData _PtrData = GetDataFromPointer(aPtr, aGfxData);
    WriteBytes<u32>(aFile, POINTER_CODE);
    _PtrData.first.Write(aFile);
    WriteBytes<u32>(aFile, _PtrData.second);
}

//
// Lights
//

static void WriteLightData(FILE* aFile, GfxData* aGfxData, DataNode<Lights1> *aNode) {
    if (!aNode->mData) return;

    // Header
    WriteBytes<u8>(aFile, DATA_TYPE_LIGHT);
    aNode->mName.Write(aFile);

    // Data
    WriteBytes<Lights1>(aFile, *aNode->mData);
}

//
// Textures
//

static void WriteTextureData(FILE* aFile, GfxData* aGfxData, DataNode<TexData> *aNode) {
    if (!aNode->mData) return;

    // Header
    WriteBytes<u8>(aFile, DATA_TYPE_TEXTURE);
    aNode->mName.Write(aFile);

    // Data
    aNode->mData->mPngData.Write(aFile);
}

//
// Vertices
//

static void WriteVertexData(FILE* aFile, GfxData* aGfxData, DataNode<Vtx> *aNode) {
    if (!aNode->mData) return;

    // Header
    WriteBytes<u8>(aFile, DATA_TYPE_VERTEX);
    aNode->mName.Write(aFile);

    // Data
    WriteBytes<u32>(aFile, aNode->mSize);
    for (u32 i = 0; i != aNode->mSize; ++i) {
        WriteBytes<s16>(aFile, aNode->mData[i].n.ob[0]);
        WriteBytes<s16>(aFile, aNode->mData[i].n.ob[1]);
        WriteBytes<s16>(aFile, aNode->mData[i].n.ob[2]);
        WriteBytes<s16>(aFile, aNode->mData[i].n.flag);
        WriteBytes<s16>(aFile, aNode->mData[i].n.tc[0]);
        WriteBytes<s16>(aFile, aNode->mData[i].n.tc[1]);
        WriteBytes<s8> (aFile, aNode->mData[i].n.n[0]);
        WriteBytes<s8> (aFile, aNode->mData[i].n.n[1]);
        WriteBytes<s8> (aFile, aNode->mData[i].n.n[2]);
        WriteBytes<u8> (aFile, aNode->mData[i].n.a);
    }
}

//
// Display lists
//

static void WriteDisplayListData(FILE *aFile, GfxData *aGfxData, DataNode<Gfx> *aNode) {
    if (!aNode->mData) return;

    // Header
    WriteBytes<u8>(aFile, DATA_TYPE_DISPLAY_LIST);
    aNode->mName.Write(aFile);

    // Data
    WriteBytes<u32>(aFile, aNode->mSize);
    for (u32 i = 0; i != aNode->mSize; ++i) {
        Gfx *_Head = &aNode->mData[i];
        if (aGfxData->mPointerList.Find((void *) _Head) != -1) {
            WriteBytes<u32>(aFile, _Head->words.w0);
            WritePointer(aFile, (const void *) _Head->words.w1, aGfxData);
        } else {
            WriteBytes<u32>(aFile, _Head->words.w0);
            WriteBytes<u32>(aFile, _Head->words.w1);
        }
    }
}

//
// Geo layouts
//

static void WriteGeoLayoutData(FILE *aFile, GfxData *aGfxData, DataNode<GeoLayout> *aNode) {
    if (!aNode->mData) return;

    // Header
    WriteBytes<u8>(aFile, DATA_TYPE_GEO_LAYOUT);
    aNode->mName.Write(aFile);

    // Data
    WriteBytes<u32>(aFile, aNode->mSize);
    for (u32 i = 0; i != aNode->mSize; ++i) {
        GeoLayout *_Head = &aNode->mData[i];
        if (aGfxData->mPointerList.Find((void *) _Head) != -1) {
            WritePointer(aFile, (const void *) (*_Head), aGfxData);
        } else {
            WriteBytes<u32>(aFile, *((u32 *) _Head));
        }
    }
}

//
// Animation data
//

static void WriteAnimationData(FILE* aFile, GfxData* aGfxData) {
    for (auto& _Node : aGfxData->mAnimations) {

        // Value buffer
        s32 _ValueBufferIdx = aGfxData->mAnimValues.FindIf([&_Node](const AnimBuffer<s16> *aAnimBuffer) { return aAnimBuffer->first == _Node->mData->mValues.first; });
        if (_ValueBufferIdx == -1) {
            continue;
        }

        // Index buffer
        s32 _IndexBufferIdx = aGfxData->mAnimIndices.FindIf([&_Node](const AnimBuffer<u16> *aAnimBuffer) { return aAnimBuffer->first == _Node->mData->mIndex.first; });
        if (_IndexBufferIdx == -1) {
            continue;
        }

        // Unk0A buffer
        s32 _Unk0ABufferIdx = aGfxData->mAnimIndices.FindIf([&_Node](const AnimBuffer<u16> *aAnimBuffer) { return aAnimBuffer->first == _Node->mData->mUnk0A.first; });
        if (_Unk0ABufferIdx == -1) {
            continue;
        }

        // Header
        WriteBytes<u8>(aFile, DATA_TYPE_ANIMATION);
        _Node->mName.Write(aFile);

        // Data
        WriteBytes<s16>(aFile, _Node->mData->mFlags);
        WriteBytes<s16>(aFile, _Node->mData->mUnk02);
        WriteBytes<s16>(aFile, _Node->mData->mUnk04);
        WriteBytes<s16>(aFile, _Node->mData->mUnk06);
        WriteBytes<s16>(aFile, _Node->mData->mUnk08);
        WriteBytes<s16>(aFile, (aGfxData->mAnimIndices[_Unk0ABufferIdx]->second.Count() / 6) - 1);
        WriteBytes<u32>(aFile, _Node->mData->mLength);
        aGfxData->mAnimValues[_ValueBufferIdx]->second.Write(aFile);
        aGfxData->mAnimIndices[_IndexBufferIdx]->second.Write(aFile);
    }
}

//
// Animation table
//

static void WriteAnimationTable(FILE* aFile, GfxData* aGfxData) {
    for (auto& _AnimName : aGfxData->mAnimationTable) {

        // Header
        WriteBytes<u8>(aFile, DATA_TYPE_ANIMATION_TABLE);

        // Data
        _AnimName.first.Write(aFile);
    }
}

//
// Write
//

bool DynOS_Gfx_WriteBinary(const SysPath &aOutputFilename, GfxData *aGfxData) {
    FILE *_File = fopen(aOutputFilename.c_str(), "wb");
    if (!_File) {
        PrintError("  ERROR: Unable to create file \"%s\"", aOutputFilename.c_str());
        return false;
    }

    for (u64 i = 0; i != aGfxData->mLoadIndex; ++i) {
        for (auto &_Node : aGfxData->mLights) {
            if (_Node->mLoadIndex == i) {
                WriteLightData(_File, aGfxData, _Node);
            }
        }
        for (auto &_Node : aGfxData->mTextures) {
            if (_Node->mLoadIndex == i) {
                WriteTextureData(_File, aGfxData, _Node);
            }
        }
        for (auto &_Node : aGfxData->mVertices) {
            if (_Node->mLoadIndex == i) {
                WriteVertexData(_File, aGfxData, _Node);
            }
        }
        for (auto &_Node : aGfxData->mDisplayLists) {
            if (_Node->mLoadIndex == i) {
                WriteDisplayListData(_File, aGfxData, _Node);
            }
        }
        for (auto &_Node : aGfxData->mGeoLayouts) {
            if (_Node->mLoadIndex == i) {
                WriteGeoLayoutData(_File, aGfxData, _Node);
            }
        }
    }
    WriteAnimationData(_File, aGfxData);
    WriteAnimationTable(_File, aGfxData);
    fclose(_File);
    return true;
}

//
// Free
//

void DynOS_Gfx_Free(GfxData* aGfxData) {
    if (aGfxData) {
        for (auto& _Node : aGfxData->mLights) {
            Delete(_Node->mData);
            Delete(_Node);
        }
        for (auto& _Node : aGfxData->mTextures) {
            Delete(_Node->mData);
            Delete(_Node);
        }
        for (auto& _Node : aGfxData->mVertices) {
            Delete(_Node->mData);
            Delete(_Node);
        }
        for (auto& _Node : aGfxData->mDisplayLists) {
            Delete(_Node->mData);
            Delete(_Node);
        }
        for (auto& _Node : aGfxData->mGeoLayouts) {
            Delete(_Node->mData);
            Delete(_Node);
        }
        for (auto& _Node : aGfxData->mAnimations) {
            Delete(_Node->mData);
            Delete(_Node);
        }
        Delete(aGfxData);
    }
}
