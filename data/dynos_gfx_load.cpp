#include "dynos.cpp.h"

//
// Pointers
//

static void *GetPointerFromData(GfxData *aGfxData, const String &aPtrName, u32 aPtrData) {

    // Lights
    for (auto& _Node : aGfxData->mLights) {
        if (_Node->mName == aPtrName) {
            if (aPtrData == 1) {
                return (void *) &_Node->mData->l[0];
            }
            if (aPtrData == 2) {
                return (void *) &_Node->mData->a;
            }
            sys_fatal("Unknown Light type: %u", aPtrData);
        }
    }

    // Textures
    for (auto& _Node : aGfxData->mTextures) {
        if (_Node->mName == aPtrName) {
            return (void *) _Node;
        }
    }

    // Display lists
    for (auto &_Node : aGfxData->mDisplayLists) {
        if (_Node->mName == aPtrName) {
            return (void *) _Node->mData;
        }
    }

    // Geo layouts
    for (auto &_Node : aGfxData->mGeoLayouts) {
        if (_Node->mName == aPtrName) {
            return (void *) _Node->mData;
        }
    }

    // Vertices
    for (auto &_Node : aGfxData->mVertices) {
        if (_Node->mName == aPtrName) {
            return (void *) (_Node->mData + aPtrData);
        }
    }

    // Error
    sys_fatal("Pointer not found: %s", aPtrName.begin());
    return NULL;
}

static void *ReadPointer(FILE *aFile, GfxData *aGfxData, u32 aValue) {

    // FUNC
    if (aValue == FUNCTION_CODE) {
        s32 _GeoFunctionIndex = ReadBytes<s32>(aFile);
        return DynOS_Geo_GetFunctionPointerFromIndex(_GeoFunctionIndex);
    }

    // PNTR
    if (aValue == POINTER_CODE) {
        String _PtrName; _PtrName.Read(aFile);
        u32   _PtrData = ReadBytes<u32>(aFile);
        return GetPointerFromData(aGfxData, _PtrName, _PtrData);
    }

    // Not a pointer
    return NULL;
}

//
// Read binary
//

static void LoadLightData(FILE *aFile, GfxData *aGfxData) {
    DataNode<Lights1> *_Node = New<DataNode<Lights1>>();

    // Name
    _Node->mName.Read(aFile);

    // Data
    _Node->mData = New<Lights1>();
    *_Node->mData = ReadBytes<Lights1>(aFile);

    // Append
    aGfxData->mLights.Add(_Node);
}

static void LoadTextureData(FILE *aFile, GfxData *aGfxData) {
    DataNode<TexData> *_Node = New<DataNode<TexData>>();

    // Name
    _Node->mName.Read(aFile);

    // Data
    _Node->mData = New<TexData>();
    _Node->mData->mUploaded = false;
    _Node->mData->mPngData.Read(aFile);
    if (!_Node->mData->mPngData.Empty()) {
        u8 *_RawData = stbi_load_from_memory(_Node->mData->mPngData.begin(), _Node->mData->mPngData.Count(), &_Node->mData->mRawWidth, &_Node->mData->mRawHeight, NULL, 4);
        _Node->mData->mRawFormat = G_IM_FMT_RGBA;
        _Node->mData->mRawSize   = G_IM_SIZ_32b;
        _Node->mData->mRawData   = Array<u8>(_RawData, _RawData + (_Node->mData->mRawWidth * _Node->mData->mRawHeight * 4));
        free(_RawData);
    } else { // Probably a palette
        _Node->mData->mRawData   = Array<u8>();
        _Node->mData->mRawWidth  = 0;
        _Node->mData->mRawHeight = 0;
        _Node->mData->mRawFormat = 0;
        _Node->mData->mRawSize   = 0;
    }

    // Append
    aGfxData->mTextures.Add(_Node);
}

static void LoadVertexData(FILE *aFile, GfxData *aGfxData) {
    DataNode<Vtx> *_Node = New<DataNode<Vtx>>();

    // Name
    _Node->mName.Read(aFile);

    // Data
    _Node->mSize = ReadBytes<u32>(aFile);
    _Node->mData = New<Vtx>(_Node->mSize);
    for (u32 i = 0; i != _Node->mSize; ++i) {
        _Node->mData[i].n.ob[0] = ReadBytes<s16>(aFile);
        _Node->mData[i].n.ob[1] = ReadBytes<s16>(aFile);
        _Node->mData[i].n.ob[2] = ReadBytes<s16>(aFile);
        _Node->mData[i].n.flag  = ReadBytes<s16>(aFile);
        _Node->mData[i].n.tc[0] = ReadBytes<s16>(aFile);
        _Node->mData[i].n.tc[1] = ReadBytes<s16>(aFile);
        _Node->mData[i].n.n[0]  = ReadBytes<s8> (aFile);
        _Node->mData[i].n.n[1]  = ReadBytes<s8> (aFile);
        _Node->mData[i].n.n[2]  = ReadBytes<s8> (aFile);
        _Node->mData[i].n.a     = ReadBytes<u8>(aFile);
    }

    // Append
    aGfxData->mVertices.Add(_Node);
}

static void LoadDisplayListData(FILE *aFile, GfxData *aGfxData) {
    DataNode<Gfx> *_Node = New<DataNode<Gfx>>();

    // Name
    _Node->mName.Read(aFile);

    // Data
    _Node->mSize = ReadBytes<u32>(aFile);
    _Node->mData = New<Gfx>(_Node->mSize);
    for (u32 i = 0; i != _Node->mSize; ++i) {
        u32 _WordsW0 = ReadBytes<u32>(aFile);
        u32 _WordsW1 = ReadBytes<u32>(aFile);
        void *_Ptr = ReadPointer(aFile, aGfxData, _WordsW1);
        if (_Ptr) {
            _Node->mData[i].words.w0 = (uintptr_t) _WordsW0;
            _Node->mData[i].words.w1 = (uintptr_t) _Ptr;
        } else {
            _Node->mData[i].words.w0 = (uintptr_t) _WordsW0;
            _Node->mData[i].words.w1 = (uintptr_t) _WordsW1;
        }
    }

    // Append
    aGfxData->mDisplayLists.Add(_Node);
}

static void LoadGeoLayoutData(FILE *aFile, GfxData *aGfxData) {
    DataNode<GeoLayout> *_Node = New<DataNode<GeoLayout>>();

    // Name
    _Node->mName.Read(aFile);

    // Data
    _Node->mSize = ReadBytes<u32>(aFile);
    _Node->mData = New<GeoLayout>(_Node->mSize);
    for (u32 i = 0; i != _Node->mSize; ++i) {
        u32 _Value = ReadBytes<u32>(aFile);
        void *_Ptr = ReadPointer(aFile, aGfxData, _Value);
        if (_Ptr) {
            _Node->mData[i] = (uintptr_t) _Ptr;
        } else {
            _Node->mData[i] = (uintptr_t) _Value;
        }
    }

    // Append
    aGfxData->mGeoLayouts.Add(_Node);
}

// For retro-compatibility
static void LoadGfxDynCmd(FILE *aFile, GfxData *aGfxData) {
    Gfx *_Data = NULL;
    String _DisplayListName; _DisplayListName.Read(aFile);
    for (auto& _DisplayList : aGfxData->mDisplayLists) {
        if (_DisplayList->mName == _DisplayListName) {
            _Data = _DisplayList->mData;
            break;
        }
    }
    if (!_Data) {
        sys_fatal("Display list not found: %s", _DisplayListName.begin());
    }
    ReadBytes<u32>(aFile);
    ReadBytes<u8>(aFile);
}

static void LoadAnimationData(FILE *aFile, GfxData *aGfxData) {
    DataNode<AnimData> *_Node = New<DataNode<AnimData>>();

    // Name
    _Node->mName.Read(aFile);

    // Data
    _Node->mData = New<AnimData>();
    _Node->mData->mFlags = ReadBytes<s16>(aFile);
    _Node->mData->mUnk02 = ReadBytes<s16>(aFile);
    _Node->mData->mUnk04 = ReadBytes<s16>(aFile);
    _Node->mData->mUnk06 = ReadBytes<s16>(aFile);
    _Node->mData->mUnk08 = ReadBytes<s16>(aFile);
    _Node->mData->mUnk0A.second = ReadBytes<s16>(aFile);
    _Node->mData->mLength = ReadBytes<u32>(aFile);
    _Node->mData->mValues.second.Read(aFile);
    _Node->mData->mIndex.second.Read(aFile);

    // Append
    aGfxData->mAnimations.Add(_Node);
}

static void LoadAnimationTable(FILE *aFile, GfxData *aGfxData) {
    void *_AnimationPtr = NULL;

    // Data
    String _AnimationName; _AnimationName.Read(aFile);
    if (_AnimationName != "NULL") {
        for (auto &_AnimData : aGfxData->mAnimations) {
            if (_AnimData->mName == _AnimationName) {
                _AnimationPtr = (void *) _AnimData->mData;
                break;
            }
        }
        if (!_AnimationPtr) {
            sys_fatal("Animation not found: %s", _AnimationName.begin());
        }
    }

    // Append
    aGfxData->mAnimationTable.Add({ "", _AnimationPtr });
}

GfxData *DynOS_Gfx_LoadFromBinary(const SysPath &aFilename) {
    FILE *_File = fopen(aFilename.c_str(), "rb");
    if (!_File) {
        return NULL;
    }

    // Data
    GfxData *_GfxData = New<GfxData>();
    for (bool _Done = false; !_Done;) {
        switch (ReadBytes<u8>(_File)) {
            case DATA_TYPE_LIGHT:           LoadLightData      (_File, _GfxData); break;
            case DATA_TYPE_TEXTURE:         LoadTextureData    (_File, _GfxData); break;
            case DATA_TYPE_VERTEX:          LoadVertexData     (_File, _GfxData); break;
            case DATA_TYPE_DISPLAY_LIST:    LoadDisplayListData(_File, _GfxData); break;
            case DATA_TYPE_GEO_LAYOUT:      LoadGeoLayoutData  (_File, _GfxData); break;
            case DATA_TYPE_ANIMATION:       LoadAnimationData  (_File, _GfxData); break;
            case DATA_TYPE_ANIMATION_TABLE: LoadAnimationTable (_File, _GfxData); break;
            case DATA_TYPE_GFXDYNCMD:       LoadGfxDynCmd      (_File, _GfxData); break;
            default:                        _Done = true;                         break;
        }
    }
    fclose(_File);
    return _GfxData;
}
