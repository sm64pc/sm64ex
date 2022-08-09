#include "dynos.cpp.h"
extern "C" {
#include "geo_commands.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
}

#define LAYER_FORCE                     0
#define LAYER_OPAQUE                    1
#define LAYER_OPAQUE_DECAL              2
#define LAYER_OPAQUE_INTER              3
#define LAYER_ALPHA                     4
#define LAYER_TRANSPARENT               5
#define LAYER_TRANSPARENT_DECAL         6
#define LAYER_TRANSPARENT_INTER         7
#define DISPLAY_LIST_SIZE_PER_TOKEN     4
#define GEO_LAYOUT_SIZE_PER_TOKEN       4

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"

static DataNode<Gfx>* ParseDisplayListData(GfxData* aGfxData, DataNode<Gfx>* aNode);
static DataNode<GeoLayout>* ParseGeoLayoutData(GfxData* aGfxData, DataNode<GeoLayout>* aNode, bool aDisplayPercent);

//
// Model files
//

enum {
    COMMENT_NONE = 0,
    COMMENT_START,       // first slash
    COMMENT_SINGLE_LINE, // double slash, reset to COMMENT_NONE if \\n is hit
    COMMENT_BLOCK,       // slash-star, set to comment block end if * is hit
    COMMENT_BLOCK_END,   // slash-star-star, set to comment none if / is hit, else return to COMMENT_BLOCK
};

struct IfDefPtr { const char *mPtr; u64 mSize; bool mErase; };
static IfDefPtr GetNearestIfDefPointer(char *pFileBuffer) {
    static const IfDefPtr sIfDefs[] = {
        { "#ifdef VERSION_JP",  17, true  },
        { "#ifndef VERSION_JP", 18, false },
        { "#ifdef VERSION_EU",  17, true  },
        { "#ifdef TEXTURE_FIX", 18, false },
    };
    IfDefPtr _Nearest = { NULL, 0, false };
    for (const auto &_IfDef : sIfDefs) {
        const char *_Ptr = strstr(pFileBuffer, _IfDef.mPtr);
        if (_Ptr != NULL && (_Nearest.mPtr == NULL || _Nearest.mPtr > _Ptr)) {
            _Nearest.mPtr = _Ptr;
            _Nearest.mSize = _IfDef.mSize;
            _Nearest.mErase = _IfDef.mErase;
        }
    }
    return _Nearest;
}

static char *LoadFileBuffer(FILE* aFile, GfxData* aGfxData) {
    fseek(aFile, 0, SEEK_END);
    s32 _Length = ftell(aFile);
    if (aGfxData && aGfxData->mModelIdentifier == 0) {
        aGfxData->mModelIdentifier = (u32) _Length;
    }

    // Remove comments
    rewind(aFile);
    char *_FileBuffer = New<char>(_Length + 1);
    char *pFileBuffer = _FileBuffer;
    char _Previous = 0;
    char _Current = 0;
    s32 _CommentType = 0;
    while (fread(&_Current, 1, 1, aFile)) {
        if (_CommentType == COMMENT_NONE) {
            if (_Current == '/') {
                _CommentType = COMMENT_START;
            } else {
                *(pFileBuffer++) = _Current;
            }
        } else if (_CommentType == COMMENT_START) {
            if (_Current == '/') {
                _CommentType = COMMENT_SINGLE_LINE;
            } else if (_Current == '*') {
                _CommentType = COMMENT_BLOCK;
            } else {
                _CommentType = COMMENT_NONE;
                *(pFileBuffer++) = _Previous;
                *(pFileBuffer++) = _Current;
            }
        } else if (_CommentType == COMMENT_SINGLE_LINE) {
            if (_Current == '\n') {
                _CommentType = COMMENT_NONE;
                *(pFileBuffer++) = _Current;
            }
        } else if (_CommentType == COMMENT_BLOCK) {
            if (_Current == '*') {
                _CommentType = COMMENT_BLOCK_END;
            }
        } else if (_CommentType == COMMENT_BLOCK_END) {
            if (_Current == '/') {
                _CommentType = COMMENT_NONE;
            } else {
                _CommentType = COMMENT_BLOCK;
            }
        }
        _Previous = _Current;
    }
    *(pFileBuffer++) = 0;

    // Remove ifdef blocks
    // Doesn't support nested blocks
    for (pFileBuffer = _FileBuffer; pFileBuffer != NULL;) {
        IfDefPtr _IfDefPtr = GetNearestIfDefPointer(pFileBuffer);
        if (_IfDefPtr.mPtr) {
            char *pIfDef = (char *) _IfDefPtr.mPtr;
            char *pElse  = (char *) strstr(_IfDefPtr.mPtr, "#else");
            char *pEndIf = (char *) strstr(_IfDefPtr.mPtr, "#endif");

            if (pElse && pElse < pEndIf) {
                if (_IfDefPtr.mErase) memset(pIfDef, ' ', pElse + 5 - pIfDef);
                else                  memset(pElse,  ' ', pEndIf - pElse);
            } else {
                if (_IfDefPtr.mErase) memset(pIfDef, ' ', pEndIf - pIfDef);
            }

            memset(pIfDef, ' ', _IfDefPtr.mSize);
            memset(pEndIf, ' ', 6);
            pFileBuffer = pEndIf;
        } else {
            pFileBuffer = NULL;
        }
    }

    return _FileBuffer;
}

template <typename T>
static void AppendNewNode(GfxData *aGfxData, DataNodes<T> &aNodes, const String &aName, String *&aDataName, Array<String> *&aDataTokens) {
    DataNode<T> *_Node = New<DataNode<T>>();
    _Node->mName = aName;
    _Node->mModelIdentifier = aGfxData->mModelIdentifier;
    aNodes.Add(_Node);
    aDataName = &_Node->mName;
    aDataTokens = &_Node->mTokens;
}

static void ScanModelFile(GfxData *aGfxData, const SysPath &aFilename) {
    FILE *_File = fopen(aFilename.c_str(), "rb");
    if (!_File) return;

    // Load file into a buffer while removing all comments
    char *_FileBuffer = LoadFileBuffer(_File, aGfxData);
    fclose(_File);

    // Scanning the loaded data
    s32 _DataType = DATA_TYPE_NONE;
    String* pDataName = NULL;
    Array<String> *pDataTokens = NULL;
    char *pDataStart = NULL;
    bool _DataIgnore = false; // Needed to ignore the '#include "file.h"' strings
    String _Buffer = "";
    for (char *c = _FileBuffer; *c != 0; ++c) {

        // Scanning data type
        if (_DataType == DATA_TYPE_NONE) {

            // Reading data type name
            if ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || (*c == '_') || (*c == '\"')) {
                if (*c == '\"') {
                    _DataIgnore = !_DataIgnore;
                } else if (!_DataIgnore) {
                    _Buffer.Add(*c);
                }
            }

            // Retrieving data type
            else if (_Buffer.Length() != 0) {
                if (_Buffer == "static") {
                    // Ignore static keyword
                } else if (_Buffer == "const") {
                    // Ignore const keyword
                } else if (_Buffer == "inline") {
                    // Ignore inline keyword
                } else if (_Buffer == "include") {
                    // Ignore include keyword
                } else if (_Buffer == "ALIGNED8") {
                    // Ignore ALIGNED8 keyword
                } else if (_Buffer == "UNUSED") {
                    // Ignore UNUSED keyword
                } else if (_Buffer == "u64") {
                    _DataType = DATA_TYPE_UNUSED;
                } else if (_Buffer == "Lights1") {
                    _DataType = DATA_TYPE_LIGHT;
                } else if (_Buffer == "u8") {
                    _DataType = DATA_TYPE_TEXTURE;
                } else if (_Buffer == "Texture") {
                    _DataType = DATA_TYPE_TEXTURE;
                } else if (_Buffer == "Vtx") {
                    _DataType = DATA_TYPE_VERTEX;
                } else if (_Buffer == "Gfx") {
                    _DataType = DATA_TYPE_DISPLAY_LIST;
                } else if (_Buffer == "GeoLayout") {
                    _DataType = DATA_TYPE_GEO_LAYOUT;
                } else {
                    PrintError("  ERROR: Unknown type name: %s", _Buffer.begin());
                }
                _Buffer.Clear();
            }
        }

        // Scanning data identifier
        else if (!pDataTokens) {

            // Reading data identifier name
            if ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || (*c == '_')) {
                _Buffer.Add(*c);
            }

            // Adding new data node
            else if (_Buffer.Length() != 0) {
                switch (_DataType) {
                    case DATA_TYPE_LIGHT:        AppendNewNode(aGfxData, aGfxData->mLights,       _Buffer, pDataName, pDataTokens); break;
                    case DATA_TYPE_TEXTURE:      AppendNewNode(aGfxData, aGfxData->mTextures,     _Buffer, pDataName, pDataTokens); break;
                    case DATA_TYPE_VERTEX:       AppendNewNode(aGfxData, aGfxData->mVertices,     _Buffer, pDataName, pDataTokens); break;
                    case DATA_TYPE_DISPLAY_LIST: AppendNewNode(aGfxData, aGfxData->mDisplayLists, _Buffer, pDataName, pDataTokens); break;
                    case DATA_TYPE_GEO_LAYOUT:   AppendNewNode(aGfxData, aGfxData->mGeoLayouts,   _Buffer, pDataName, pDataTokens); break;
                    case DATA_TYPE_UNUSED:       pDataTokens = (Array<String> *) 1;                                                 break;
                }
                _Buffer.Clear();
            }
        }

        // Looking for data
        else if (pDataStart == 0) {
            if (*c == '=') {
                pDataStart = c + 1;
            } else if (*c == ';') {
                PrintError("  ERROR: %s: Unexpected end of data", pDataName->begin());
            }
        }

        // Data end
        else if (*c == ';') {
            if (_DataType != DATA_TYPE_UNUSED) {
                char* pDataEnd = &*c;
                String _Token = "";
                for (u8 _Bracket = 0; pDataStart <= pDataEnd; pDataStart++) {
                    if (*pDataStart == '(') _Bracket++;
                    if (*pDataStart == ' ' || *pDataStart == '\t' || *pDataStart == '\r' || *pDataStart == '\n') continue;
                    if (_Bracket <= 1 && (*pDataStart == '(' || *pDataStart == ')' || *pDataStart == ',' || *pDataStart == '{' || *pDataStart == '}' || *pDataStart == ';')) {
                        if (_Token.Length() != 0) {
                            pDataTokens->Add(_Token);
                            _Token.Clear();
                        }
                    } else {
                        _Token.Add(*pDataStart);
                    }
                    if (*pDataStart == ')') _Bracket--;
                }
            }
            _DataType   = DATA_TYPE_NONE;
            pDataName   = NULL;
            pDataTokens = NULL;
            pDataStart  = NULL;
            _DataIgnore = false;
            _Buffer     = "";
        }
    }

    Delete(_FileBuffer);
    Print("Data read from file \"%s\"", aFilename.c_str());
}

//
// Lights
//

static DataNode<Lights1>* ParseLightData(GfxData* aGfxData, DataNode<Lights1>* aNode) {
    if (aNode->mData) return aNode;

    // Check tokens count
    if (aNode->mTokens.Count() < 10) {
        PrintError("  ERROR: %s: not enough data", aNode->mName.begin());
        return aNode;
    }

    // Parse def token
    if (aNode->mTokens[0] != "gdSPDefLights1") {
        PrintError("  ERROR: Invalid def token: should be gdSPDefLights1, is %s", aNode->mTokens[0].begin());
        return aNode;
    }

    // Parse data tokens
    u8 ar = (u8) aNode->mTokens[1].ParseInt();
    u8 ag = (u8) aNode->mTokens[2].ParseInt();
    u8 ab = (u8) aNode->mTokens[3].ParseInt();
    u8 r1 = (u8) aNode->mTokens[4].ParseInt();
    u8 g1 = (u8) aNode->mTokens[5].ParseInt();
    u8 b1 = (u8) aNode->mTokens[6].ParseInt();
    s8  x1 = (s8)  aNode->mTokens[7].ParseInt();
    s8  y1 = (s8)  aNode->mTokens[8].ParseInt();
    s8  z1 = (s8)  aNode->mTokens[9].ParseInt();
    aNode->mData = New<Lights1>();
    *aNode->mData = gdSPDefLights1(ar, ag, ab, r1, g1, b1, x1, y1, z1);
    aNode->mLoadIndex = aGfxData->mLoadIndex++;
    return aNode;
}

//
// Textures
//

static TexData* LoadTextureFromFile(GfxData *aGfxData, const String& aFile) {

    // Image file
    String _TexturePath = aFile.SubString(aFile.Find('/') + 1); // Remove the "actors/"
    SysPath _Filename = fstring("%s/%s.png", aGfxData->mPackFolder.c_str(), _TexturePath.begin());
    FILE *_File = fopen(_Filename.c_str(), "rb");
    if (!_File) {
        PrintError("  ERROR: Unable to open file \"%s\"", _Filename.c_str());
        return NULL;
    }

    // Texture data
    fseek(_File, 0, SEEK_END);
    TexData* _Texture = New<TexData>();
    _Texture->mPngData.Resize(ftell(_File)); rewind(_File);
    fread(_Texture->mPngData.begin(), sizeof(u8), _Texture->mPngData.Count(), _File);
    fclose(_File);
    return _Texture;
}

static void ConvertTextureDataToPng(GfxData *aGfxData, TexData* aTexture) {

    // Convert to RGBA32
    const u8 *_Palette = (aGfxData->mGfxContext.mCurrentPalette ? aGfxData->mGfxContext.mCurrentPalette->mData->mRawData.begin() : NULL);
    u8 *_Buffer = DynOS_Gfx_TextureConvertToRGBA32(aTexture->mRawData.begin(), aTexture->mRawData.Count(), aTexture->mRawFormat, aTexture->mRawSize, _Palette);
    if (_Buffer == NULL) {
        PrintError("  ERROR: Unknown texture format");
        return;
    }

    // Convert to PNG
    s32 _PngLength = 0;
    u8 *_PngData = stbi_write_png_to_mem(_Buffer, 0, aTexture->mRawWidth, aTexture->mRawHeight, 4, &_PngLength);
    if (!_PngData || !_PngLength) {
        PrintError("  ERROR: Cannot convert texture to PNG");
        return;
    }

    aTexture->mPngData = Array<u8>(_PngData, _PngData + _PngLength);
    Delete(_PngData);
}

static DataNode<TexData>* ParseTextureData(GfxData* aGfxData, DataNode<TexData>* aNode) {
    if (aNode->mData) return aNode;

    // Check tokens Count
    if (aNode->mTokens.Count() < 1) {
        PrintError("  ERROR: %s: not enough data", aNode->mName.begin());
        return aNode;
    }

    // #include"[texture].inc.c"
    s32 i0 = aNode->mTokens[0].Find("#include");
    if (i0 != -1) {
        s32 i1 = aNode->mTokens[0].Find(".inc.c");
        if (i1 == -1) {
            PrintError("  ERROR: %s: missing .inc.c in String %s", aNode->mName.begin(), aNode->mTokens[0].begin());
            return aNode;
        }

        // Filename
        String _Filename  = aNode->mTokens[0].SubString(i0 + 9, i1 - i0 - 9);
        aNode->mData      = LoadTextureFromFile(aGfxData, _Filename);
        aNode->mLoadIndex = aGfxData->mLoadIndex++;
        return aNode;
    }

    // double quoted String
    s32 dq0 = aNode->mTokens[0].Find('\"');
    if (dq0 != -1) {
        s32 dq1 = aNode->mTokens[0].Find('\"', dq0 + 1);
        if (dq1 == -1) {
            PrintError("  ERROR: %s: missing second quote in String %s", aNode->mName.begin(), aNode->mTokens[0].begin());
            return aNode;
        }

        // Filename
        String _Filename  = aNode->mTokens[0].SubString(dq0 + 1, dq1 - dq0 - 1);
        aNode->mData      = LoadTextureFromFile(aGfxData, _Filename);
        aNode->mLoadIndex = aGfxData->mLoadIndex++;
        return aNode;
    }

    // Stream of bytes
    aNode->mData              = New<TexData>();
    aNode->mData->mRawWidth   = -1; // Unknown for now, will be set later
    aNode->mData->mRawHeight  = -1; // Unknown for now, will be set later
    aNode->mData->mRawFormat  = -1; // Unknown for now, will be set later
    aNode->mData->mRawSize    = -1; // Unknown for now, will be set later
    aNode->mData->mRawData.Resize(aNode->mTokens.Count());
    for (u64 j = 0; j != aNode->mTokens.Count(); ++j) {
    aNode->mData->mRawData[j] = aNode->mTokens[j].ParseInt();
    }
    aNode->mLoadIndex         = aGfxData->mLoadIndex++;
    return aNode;
}

//
// Vertices
//

static DataNode<Vtx>* ParseVertexData(GfxData* aGfxData, DataNode<Vtx>* aNode) {
    if (aNode->mData) return aNode;

    // Vertex data
    aNode->mSize = (u32) (aNode->mTokens.Count() / 10);
    aNode->mData = New<Vtx>(aNode->mSize);
    for (u32 i = 0; i != aNode->mSize; ++i) {
        f32 px = (f32) aNode->mTokens[10 * i + 0].ParseFloat();
        f32 py = (f32) aNode->mTokens[10 * i + 1].ParseFloat();
        f32 pz = (f32) aNode->mTokens[10 * i + 2].ParseFloat();
        u8 fl = (u8) aNode->mTokens[10 * i + 3].ParseInt();
        s16 tu = (s16) aNode->mTokens[10 * i + 4].ParseInt();
        s16 tv = (s16) aNode->mTokens[10 * i + 5].ParseInt();
        u8 nx = (u8) aNode->mTokens[10 * i + 6].ParseInt();
        u8 ny = (u8) aNode->mTokens[10 * i + 7].ParseInt();
        u8 nz = (u8) aNode->mTokens[10 * i + 8].ParseInt();
        u8 a  = (u8) aNode->mTokens[10 * i + 9].ParseInt();
        aNode->mData[i] = { { { px, py, pz }, fl, { tu, tv }, { nx, ny, nz, a } } };
    }
    aNode->mLoadIndex = aGfxData->mLoadIndex++;
    return aNode;
}

//
// Display lists
//

#define gfx_constant(x) if (_Arg == #x) { return (s64) (x); }
static s64 ParseGfxSymbolArg(GfxData* aGfxData, DataNode<Gfx>* aNode, u64* pTokenIndex, const char *aPrefix) {
    assert(aPrefix != NULL);
    String _Token = (pTokenIndex != NULL ? aNode->mTokens[(*pTokenIndex)++] : "");
    String _Arg("%s%s", aPrefix, _Token.begin());

    // Offset
    s32 _Offset = 0;
    s32 _Plus = _Arg.Find('+');
    if (_Plus != -1) {
        _Offset = _Arg.SubString(_Plus + 1).ParseInt();
        _Arg = _Arg.SubString(0, _Plus);
    }

    // Constants
    gfx_constant(NULL);
    gfx_constant(G_ON);
    gfx_constant(G_OFF);

    // Combine modes
    gfx_constant(G_CCMUX_COMBINED);
    gfx_constant(G_CCMUX_TEXEL0);
    gfx_constant(G_CCMUX_TEXEL1);
    gfx_constant(G_CCMUX_PRIMITIVE);
    gfx_constant(G_CCMUX_SHADE);
    gfx_constant(G_CCMUX_ENVIRONMENT);
    gfx_constant(G_CCMUX_CENTER);
    gfx_constant(G_CCMUX_SCALE);
    gfx_constant(G_CCMUX_COMBINED_ALPHA);
    gfx_constant(G_CCMUX_TEXEL0_ALPHA);
    gfx_constant(G_CCMUX_TEXEL1_ALPHA);
    gfx_constant(G_CCMUX_PRIMITIVE_ALPHA);
    gfx_constant(G_CCMUX_SHADE_ALPHA);
    gfx_constant(G_CCMUX_ENV_ALPHA);
    gfx_constant(G_CCMUX_LOD_FRACTION);
    gfx_constant(G_CCMUX_PRIM_LOD_FRAC);
    gfx_constant(G_CCMUX_NOISE);
    gfx_constant(G_CCMUX_K4);
    gfx_constant(G_CCMUX_K5);
    gfx_constant(G_CCMUX_1);
    gfx_constant(G_CCMUX_0);
    gfx_constant(G_ACMUX_COMBINED);
    gfx_constant(G_ACMUX_TEXEL0);
    gfx_constant(G_ACMUX_TEXEL1);
    gfx_constant(G_ACMUX_PRIMITIVE);
    gfx_constant(G_ACMUX_SHADE);
    gfx_constant(G_ACMUX_ENVIRONMENT);
    gfx_constant(G_ACMUX_LOD_FRACTION);
    gfx_constant(G_ACMUX_PRIM_LOD_FRAC);
    gfx_constant(G_ACMUX_1);
    gfx_constant(G_ACMUX_0);

    // Light constants
    gfx_constant(NUMLIGHTS_0);
    gfx_constant(NUMLIGHTS_1);
    gfx_constant(NUMLIGHTS_2);
    gfx_constant(NUMLIGHTS_3);
    gfx_constant(NUMLIGHTS_4);
    gfx_constant(NUMLIGHTS_5);
    gfx_constant(NUMLIGHTS_6);
    gfx_constant(NUMLIGHTS_7);

    // Image formats
    gfx_constant(G_IM_FMT_RGBA);
    gfx_constant(G_IM_FMT_YUV);
    gfx_constant(G_IM_FMT_CI);
    gfx_constant(G_IM_FMT_IA);
    gfx_constant(G_IM_FMT_I);
    gfx_constant(G_IM_SIZ_4b);
    gfx_constant(G_IM_SIZ_8b);
    gfx_constant(G_IM_SIZ_16b);
    gfx_constant(G_IM_SIZ_32b);
    gfx_constant(G_IM_SIZ_DD);

    // Image constants
    gfx_constant(G_IM_SIZ_4b_BYTES);
    gfx_constant(G_IM_SIZ_4b_TILE_BYTES);
    gfx_constant(G_IM_SIZ_4b_LINE_BYTES);
    gfx_constant(G_IM_SIZ_8b_BYTES);
    gfx_constant(G_IM_SIZ_8b_TILE_BYTES);
    gfx_constant(G_IM_SIZ_8b_LINE_BYTES);
    gfx_constant(G_IM_SIZ_16b_BYTES);
    gfx_constant(G_IM_SIZ_16b_TILE_BYTES);
    gfx_constant(G_IM_SIZ_16b_LINE_BYTES);
    gfx_constant(G_IM_SIZ_32b_BYTES);
    gfx_constant(G_IM_SIZ_32b_TILE_BYTES);
    gfx_constant(G_IM_SIZ_32b_LINE_BYTES);
    gfx_constant(G_IM_SIZ_4b_LOAD_BLOCK);
    gfx_constant(G_IM_SIZ_8b_LOAD_BLOCK);
    gfx_constant(G_IM_SIZ_16b_LOAD_BLOCK);
    gfx_constant(G_IM_SIZ_32b_LOAD_BLOCK);
    gfx_constant(G_IM_SIZ_4b_SHIFT);
    gfx_constant(G_IM_SIZ_8b_SHIFT);
    gfx_constant(G_IM_SIZ_16b_SHIFT);
    gfx_constant(G_IM_SIZ_32b_SHIFT);
    gfx_constant(G_IM_SIZ_4b_INCR);
    gfx_constant(G_IM_SIZ_8b_INCR);
    gfx_constant(G_IM_SIZ_16b_INCR);
    gfx_constant(G_IM_SIZ_32b_INCR);

    // Tile formats
    gfx_constant(G_TX_RENDERTILE);
    gfx_constant(G_TX_LOADTILE);
    gfx_constant(G_TX_NOMIRROR);
    gfx_constant(G_TX_WRAP);
    gfx_constant(G_TX_MIRROR);
    gfx_constant(G_TX_CLAMP);
    gfx_constant(G_TX_NOMASK);
    gfx_constant(G_TX_NOLOD);
    gfx_constant(G_TX_WRAP|G_TX_NOMIRROR);
    gfx_constant(G_TX_WRAP|G_TX_MIRROR);
    gfx_constant(G_TX_CLAMP|G_TX_NOMIRROR);
    gfx_constant(G_TX_CLAMP|G_TX_MIRROR);

    // Render modes
    gfx_constant(G_RM_AA_ZB_OPA_SURF);
    gfx_constant(G_RM_AA_ZB_OPA_SURF2);
    gfx_constant(G_RM_AA_ZB_XLU_SURF);
    gfx_constant(G_RM_AA_ZB_XLU_SURF2);
    gfx_constant(G_RM_AA_ZB_OPA_DECAL);
    gfx_constant(G_RM_AA_ZB_OPA_DECAL2);
    gfx_constant(G_RM_AA_ZB_XLU_DECAL);
    gfx_constant(G_RM_AA_ZB_XLU_DECAL2);
    gfx_constant(G_RM_AA_ZB_OPA_INTER);
    gfx_constant(G_RM_AA_ZB_OPA_INTER2);
    gfx_constant(G_RM_AA_ZB_XLU_INTER);
    gfx_constant(G_RM_AA_ZB_XLU_INTER2);
    gfx_constant(G_RM_AA_ZB_XLU_LINE);
    gfx_constant(G_RM_AA_ZB_XLU_LINE2);
    gfx_constant(G_RM_AA_ZB_DEC_LINE);
    gfx_constant(G_RM_AA_ZB_DEC_LINE2);
    gfx_constant(G_RM_AA_ZB_TEX_EDGE);
    gfx_constant(G_RM_AA_ZB_TEX_EDGE2);
    gfx_constant(G_RM_AA_ZB_TEX_INTER);
    gfx_constant(G_RM_AA_ZB_TEX_INTER2);
    gfx_constant(G_RM_AA_ZB_SUB_SURF);
    gfx_constant(G_RM_AA_ZB_SUB_SURF2);
    gfx_constant(G_RM_AA_ZB_PCL_SURF);
    gfx_constant(G_RM_AA_ZB_PCL_SURF2);
    gfx_constant(G_RM_AA_ZB_OPA_TERR);
    gfx_constant(G_RM_AA_ZB_OPA_TERR2);
    gfx_constant(G_RM_AA_ZB_TEX_TERR);
    gfx_constant(G_RM_AA_ZB_TEX_TERR2);
    gfx_constant(G_RM_AA_ZB_SUB_TERR);
    gfx_constant(G_RM_AA_ZB_SUB_TERR2);
    gfx_constant(G_RM_RA_ZB_OPA_SURF);
    gfx_constant(G_RM_RA_ZB_OPA_SURF2);
    gfx_constant(G_RM_RA_ZB_OPA_DECAL);
    gfx_constant(G_RM_RA_ZB_OPA_DECAL2);
    gfx_constant(G_RM_RA_ZB_OPA_INTER);
    gfx_constant(G_RM_RA_ZB_OPA_INTER2);
    gfx_constant(G_RM_AA_OPA_SURF);
    gfx_constant(G_RM_AA_OPA_SURF2);
    gfx_constant(G_RM_AA_XLU_SURF);
    gfx_constant(G_RM_AA_XLU_SURF2);
    gfx_constant(G_RM_AA_XLU_LINE);
    gfx_constant(G_RM_AA_XLU_LINE2);
    gfx_constant(G_RM_AA_DEC_LINE);
    gfx_constant(G_RM_AA_DEC_LINE2);
    gfx_constant(G_RM_AA_TEX_EDGE);
    gfx_constant(G_RM_AA_TEX_EDGE2);
    gfx_constant(G_RM_AA_SUB_SURF);
    gfx_constant(G_RM_AA_SUB_SURF2);
    gfx_constant(G_RM_AA_PCL_SURF);
    gfx_constant(G_RM_AA_PCL_SURF2);
    gfx_constant(G_RM_AA_OPA_TERR);
    gfx_constant(G_RM_AA_OPA_TERR2);
    gfx_constant(G_RM_AA_TEX_TERR);
    gfx_constant(G_RM_AA_TEX_TERR2);
    gfx_constant(G_RM_AA_SUB_TERR);
    gfx_constant(G_RM_AA_SUB_TERR2);
    gfx_constant(G_RM_RA_OPA_SURF);
    gfx_constant(G_RM_RA_OPA_SURF2);
    gfx_constant(G_RM_ZB_OPA_SURF);
    gfx_constant(G_RM_ZB_OPA_SURF2);
    gfx_constant(G_RM_ZB_XLU_SURF);
    gfx_constant(G_RM_ZB_XLU_SURF2);
    gfx_constant(G_RM_ZB_OPA_DECAL);
    gfx_constant(G_RM_ZB_OPA_DECAL2);
    gfx_constant(G_RM_ZB_XLU_DECAL);
    gfx_constant(G_RM_ZB_XLU_DECAL2);
    gfx_constant(G_RM_ZB_CLD_SURF);
    gfx_constant(G_RM_ZB_CLD_SURF2);
    gfx_constant(G_RM_ZB_OVL_SURF);
    gfx_constant(G_RM_ZB_OVL_SURF2);
    gfx_constant(G_RM_ZB_PCL_SURF);
    gfx_constant(G_RM_ZB_PCL_SURF2);
    gfx_constant(G_RM_OPA_SURF);
    gfx_constant(G_RM_OPA_SURF2);
    gfx_constant(G_RM_XLU_SURF);
    gfx_constant(G_RM_XLU_SURF2);
    gfx_constant(G_RM_CLD_SURF);
    gfx_constant(G_RM_CLD_SURF2);
    gfx_constant(G_RM_TEX_EDGE);
    gfx_constant(G_RM_TEX_EDGE2);
    gfx_constant(G_RM_PCL_SURF);
    gfx_constant(G_RM_PCL_SURF2);
    gfx_constant(G_RM_ADD);
    gfx_constant(G_RM_ADD2);
    gfx_constant(G_RM_NOOP);
    gfx_constant(G_RM_NOOP2);
    gfx_constant(G_RM_VISCVG);
    gfx_constant(G_RM_VISCVG2);
    gfx_constant(G_RM_OPA_CI);
    gfx_constant(G_RM_OPA_CI2);
    gfx_constant(G_RM_CUSTOM_AA_ZB_XLU_SURF);
    gfx_constant(G_RM_CUSTOM_AA_ZB_XLU_SURF2);
    gfx_constant(G_RM_FOG_SHADE_A);
    gfx_constant(G_RM_FOG_PRIM_A);
    gfx_constant(G_RM_PASS);

    // Geometry modes
    gfx_constant(G_ZBUFFER);
    gfx_constant(G_SHADE);
    gfx_constant(G_TEXTURE_ENABLE);
    gfx_constant(G_SHADING_SMOOTH);
    gfx_constant(G_CULL_FRONT);
    gfx_constant(G_CULL_BACK);
    gfx_constant(G_CULL_BOTH);
    gfx_constant(G_FOG);
    gfx_constant(G_LIGHTING);
    gfx_constant(G_TEXTURE_GEN);
    gfx_constant(G_TEXTURE_GEN_LINEAR);
    gfx_constant(G_LOD);
    gfx_constant(G_CLIPPING);
    gfx_constant(G_FOG|G_TEXTURE_GEN);
    gfx_constant(G_LIGHTING|G_CULL_BACK);
    gfx_constant(G_LIGHTING|G_SHADING_SMOOTH);
    gfx_constant(G_CULL_BACK|G_SHADING_SMOOTH);
    gfx_constant(G_LIGHTING|G_CULL_BACK|G_SHADING_SMOOTH);
    gfx_constant(G_TEXTURE_GEN|G_SHADING_SMOOTH);
    gfx_constant(G_TEXTURE_GEN|G_LIGHTING|G_CULL_BACK);
    gfx_constant(G_TEXTURE_GEN|G_CULL_BACK|G_SHADING_SMOOTH);

    // Alpha modes
    gfx_constant(G_AC_NONE);
    gfx_constant(G_AC_THRESHOLD);
    gfx_constant(G_AC_DITHER);

    // Other modes
    gfx_constant(G_MDSFT_ALPHACOMPARE);
    gfx_constant(G_MDSFT_ZSRCSEL);
    gfx_constant(G_MDSFT_RENDERMODE);
    gfx_constant(G_MDSFT_BLENDER);
    gfx_constant(G_MDSFT_BLENDMASK);
    gfx_constant(G_MDSFT_ALPHADITHER);
    gfx_constant(G_MDSFT_RGBDITHER);
    gfx_constant(G_MDSFT_COMBKEY);
    gfx_constant(G_MDSFT_TEXTCONV);
    gfx_constant(G_MDSFT_TEXTFILT);
    gfx_constant(G_MDSFT_TEXTLUT);
    gfx_constant(G_MDSFT_TEXTLOD);
    gfx_constant(G_MDSFT_TEXTDETAIL);
    gfx_constant(G_MDSFT_TEXTPERSP);
    gfx_constant(G_MDSFT_CYCLETYPE);
    gfx_constant(G_MDSFT_COLORDITHER);
    gfx_constant(G_MDSFT_PIPELINE);
    gfx_constant(G_PM_1PRIMITIVE);
    gfx_constant(G_PM_NPRIMITIVE);
    gfx_constant(G_CYC_1CYCLE);
    gfx_constant(G_CYC_2CYCLE);
    gfx_constant(G_CYC_COPY);
    gfx_constant(G_CYC_FILL);
    gfx_constant(G_TP_NONE);
    gfx_constant(G_TP_PERSP);
    gfx_constant(G_TD_CLAMP);
    gfx_constant(G_TD_SHARPEN);
    gfx_constant(G_TD_DETAIL);
    gfx_constant(G_TL_TILE);
    gfx_constant(G_TL_LOD);
    gfx_constant(G_TT_NONE);
    gfx_constant(G_TT_RGBA16);
    gfx_constant(G_TT_IA16);
    gfx_constant(G_TF_POINT);
    gfx_constant(G_TF_AVERAGE);
    gfx_constant(G_TF_BILERP);
    gfx_constant(G_TC_CONV);
    gfx_constant(G_TC_FILTCONV);
    gfx_constant(G_TC_FILT);
    gfx_constant(G_CK_NONE);
    gfx_constant(G_CK_KEY);
    gfx_constant(G_CD_MAGICSQ);
    gfx_constant(G_CD_BAYER);
    gfx_constant(G_CD_NOISE);
    gfx_constant(G_CD_DISABLE);
    gfx_constant(G_CD_ENABLE);
    gfx_constant(G_AD_PATTERN);
    gfx_constant(G_AD_NOTPATTERN);
    gfx_constant(G_AD_NOISE);
    gfx_constant(G_AD_DISABLE);
    gfx_constant(G_AC_NONE);
    gfx_constant(G_AC_THRESHOLD);
    gfx_constant(G_AC_DITHER);
    gfx_constant(G_ZS_PIXEL);
    gfx_constant(G_ZS_PRIM);

    // Common values
    gfx_constant((4-1)<<G_TEXTURE_IMAGE_FRAC);
    gfx_constant((8-1)<<G_TEXTURE_IMAGE_FRAC);
    gfx_constant((16-1)<<G_TEXTURE_IMAGE_FRAC);
    gfx_constant((32-1)<<G_TEXTURE_IMAGE_FRAC);
    gfx_constant((64-1)<<G_TEXTURE_IMAGE_FRAC);
    gfx_constant((128-1)<<G_TEXTURE_IMAGE_FRAC);
    gfx_constant((256-1)<<G_TEXTURE_IMAGE_FRAC);
    gfx_constant(CALC_DXT(4,G_IM_SIZ_4b_BYTES));
    gfx_constant(CALC_DXT(8,G_IM_SIZ_4b_BYTES));
    gfx_constant(CALC_DXT(16,G_IM_SIZ_4b_BYTES));
    gfx_constant(CALC_DXT(32,G_IM_SIZ_4b_BYTES));
    gfx_constant(CALC_DXT(64,G_IM_SIZ_4b_BYTES));
    gfx_constant(CALC_DXT(128,G_IM_SIZ_4b_BYTES));
    gfx_constant(CALC_DXT(256,G_IM_SIZ_4b_BYTES));
    gfx_constant(CALC_DXT(4,G_IM_SIZ_8b_BYTES));
    gfx_constant(CALC_DXT(8,G_IM_SIZ_8b_BYTES));
    gfx_constant(CALC_DXT(16,G_IM_SIZ_8b_BYTES));
    gfx_constant(CALC_DXT(32,G_IM_SIZ_8b_BYTES));
    gfx_constant(CALC_DXT(64,G_IM_SIZ_8b_BYTES));
    gfx_constant(CALC_DXT(128,G_IM_SIZ_8b_BYTES));
    gfx_constant(CALC_DXT(256,G_IM_SIZ_8b_BYTES));
    gfx_constant(CALC_DXT(4,G_IM_SIZ_16b_BYTES));
    gfx_constant(CALC_DXT(8,G_IM_SIZ_16b_BYTES));
    gfx_constant(CALC_DXT(16,G_IM_SIZ_16b_BYTES));
    gfx_constant(CALC_DXT(32,G_IM_SIZ_16b_BYTES));
    gfx_constant(CALC_DXT(64,G_IM_SIZ_16b_BYTES));
    gfx_constant(CALC_DXT(128,G_IM_SIZ_16b_BYTES));
    gfx_constant(CALC_DXT(256,G_IM_SIZ_16b_BYTES));
    gfx_constant(CALC_DXT(4,G_IM_SIZ_32b_BYTES));
    gfx_constant(CALC_DXT(8,G_IM_SIZ_32b_BYTES));
    gfx_constant(CALC_DXT(16,G_IM_SIZ_32b_BYTES));
    gfx_constant(CALC_DXT(32,G_IM_SIZ_32b_BYTES));
    gfx_constant(CALC_DXT(64,G_IM_SIZ_32b_BYTES));
    gfx_constant(CALC_DXT(128,G_IM_SIZ_32b_BYTES));
    gfx_constant(CALC_DXT(256,G_IM_SIZ_32b_BYTES));

    // Lights
    for (auto& _Node : aGfxData->mLights) {

        // Light pointer
        if (_Arg == _Node->mName) {
            return (s64) ParseLightData(aGfxData, _Node)->mData;
        }

        // Ambient pointer
        String _Ambient("&%s.a", _Node->mName.begin());
        if (_Arg == _Ambient) {
            return (s64) &(ParseLightData(aGfxData, _Node)->mData->a);
        }

        // Diffuse pointer
        String _Diffuse("&%s.l", _Node->mName.begin());
        if (_Arg == _Diffuse) {
            return (s64) &(ParseLightData(aGfxData, _Node)->mData->l[0]);
        }
    }

    // Textures
    for (auto& _Node : aGfxData->mTextures) {
        if (_Arg == _Node->mName) {
            return (s64) ParseTextureData(aGfxData, _Node);
        }
    }

    // Vertex arrays
    for (auto& _Node : aGfxData->mVertices) {
        if (_Arg == _Node->mName) {
            return (s64) (ParseVertexData(aGfxData, _Node)->mData + _Offset);
        }
    }

    // Display lists
    for (auto& _Node : aGfxData->mDisplayLists) {
        if (_Arg == _Node->mName) {
            return (s64) ParseDisplayListData(aGfxData, _Node);
        }
    }

    // Integers
    s32 x;
    if ((_Arg[1] == 'x' && sscanf(_Arg.begin(), "%x", &x) == 1) || (sscanf(_Arg.begin(), "%d", &x) == 1)) {
        return (s64) x;
    }

    // Unknown
    PrintError("  ERROR: Unknown gfx arg: %s", _Arg.begin());
    return 0;
}

#define gfx_symbol_0(symb)                                                                             \
    if (_Symbol == #symb) {                                                                       \
        Gfx _Gfx[] = { symb() };                                                                        \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_symbol_1(symb, ptr)                                                                        \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        if (ptr) { aGfxData->mPointerList.Add(aHead); }                                         \
        Gfx _Gfx[] = { symb(_Arg0) };                                                                    \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_symbol_2(symb, ptr)                                                                        \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        if (ptr) { aGfxData->mPointerList.Add(aHead); }                                         \
        Gfx _Gfx[] = { symb(_Arg0, _Arg1) };                                                              \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_symbol_3(symb, ptr)                                                                        \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        if (ptr) { aGfxData->mPointerList.Add(aHead); }                                         \
        Gfx _Gfx[] = { symb(_Arg0, _Arg1, _Arg2) };                                                        \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_symbol_4(symb)                                                                             \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        Gfx _Gfx[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3) };                                                  \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_symbol_5(symb)                                                                             \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        Gfx _Gfx[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4) };                                            \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_symbol_6(symb)                                                                             \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg5 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        Gfx _Gfx[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4, _Arg5) };                                      \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_symbol_7(symb)                                                                             \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg5 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg6 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        Gfx _Gfx[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4, _Arg5, _Arg6) };                                \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_symbol_8(symb)                                                                             \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg5 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg6 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        s64 _Arg7 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");                                \
        Gfx _Gfx[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4, _Arg5, _Arg6, _Arg7) };                          \
        memcpy(aHead, _Gfx, sizeof(_Gfx));                                                               \
        aHead += (sizeof(_Gfx) / sizeof(_Gfx[0]));                                                       \
        return;                                                                                        \
    }

#define gfx_arg_with_suffix(argname, suffix)                                                           \
    const String& argname##_token = aNode->mTokens[aTokenIndex];                                    \
    String _Token##suffix = String("%s%s", argname##_token.begin(), #suffix);                                   \
    s64 argname = ParseGfxSymbolArg(aGfxData, aNode, NULL, _Token##suffix.begin());                         \

#define STR_VALUE_2(...) #__VA_ARGS__
#define STR_VALUE(...) STR_VALUE_2(__VA_ARGS__)
#define gfx_set_combine_mode_arg(x) if (_Arg == #x) { return String("%s", STR_VALUE(x)); }
static String ConvertSetCombineModeArgToString(GfxData *aGfxData, const String& _Arg) {
    gfx_set_combine_mode_arg(G_CC_PRIMITIVE);
    gfx_set_combine_mode_arg(G_CC_SHADE);
    gfx_set_combine_mode_arg(G_CC_MODULATEI);
    gfx_set_combine_mode_arg(G_CC_MODULATEIDECALA);
    gfx_set_combine_mode_arg(G_CC_MODULATEIFADE);
    gfx_set_combine_mode_arg(G_CC_MODULATERGB);
    gfx_set_combine_mode_arg(G_CC_MODULATERGBDECALA);
    gfx_set_combine_mode_arg(G_CC_MODULATERGBFADE);
    gfx_set_combine_mode_arg(G_CC_MODULATEIA);
    gfx_set_combine_mode_arg(G_CC_MODULATEIFADEA);
    gfx_set_combine_mode_arg(G_CC_MODULATEFADE);
    gfx_set_combine_mode_arg(G_CC_MODULATERGBA);
    gfx_set_combine_mode_arg(G_CC_MODULATERGBFADEA);
    gfx_set_combine_mode_arg(G_CC_MODULATEI_PRIM);
    gfx_set_combine_mode_arg(G_CC_MODULATEIA_PRIM);
    gfx_set_combine_mode_arg(G_CC_MODULATEIDECALA_PRIM);
    gfx_set_combine_mode_arg(G_CC_MODULATERGB_PRIM);
    gfx_set_combine_mode_arg(G_CC_MODULATERGBA_PRIM);
    gfx_set_combine_mode_arg(G_CC_MODULATERGBDECALA_PRIM);
    gfx_set_combine_mode_arg(G_CC_FADE);
    gfx_set_combine_mode_arg(G_CC_FADEA);
    gfx_set_combine_mode_arg(G_CC_DECALRGB);
    gfx_set_combine_mode_arg(G_CC_DECALRGBA);
    gfx_set_combine_mode_arg(G_CC_DECALFADE);
    gfx_set_combine_mode_arg(G_CC_DECALFADEA);
    gfx_set_combine_mode_arg(G_CC_BLENDI);
    gfx_set_combine_mode_arg(G_CC_BLENDIA);
    gfx_set_combine_mode_arg(G_CC_BLENDIDECALA);
    gfx_set_combine_mode_arg(G_CC_BLENDRGBA);
    gfx_set_combine_mode_arg(G_CC_BLENDRGBDECALA);
    gfx_set_combine_mode_arg(G_CC_BLENDRGBFADEA);
    gfx_set_combine_mode_arg(G_CC_ADDRGB);
    gfx_set_combine_mode_arg(G_CC_ADDRGBDECALA);
    gfx_set_combine_mode_arg(G_CC_ADDRGBFADE);
    gfx_set_combine_mode_arg(G_CC_REFLECTRGB);
    gfx_set_combine_mode_arg(G_CC_REFLECTRGBDECALA);
    gfx_set_combine_mode_arg(G_CC_HILITERGB);
    gfx_set_combine_mode_arg(G_CC_HILITERGBA);
    gfx_set_combine_mode_arg(G_CC_HILITERGBDECALA);
    gfx_set_combine_mode_arg(G_CC_SHADEDECALA);
    gfx_set_combine_mode_arg(G_CC_SHADEFADEA);
    gfx_set_combine_mode_arg(G_CC_BLENDPE);
    gfx_set_combine_mode_arg(G_CC_BLENDPEDECALA);
    gfx_set_combine_mode_arg(_G_CC_BLENDPE);
    gfx_set_combine_mode_arg(_G_CC_BLENDPEDECALA);
    gfx_set_combine_mode_arg(_G_CC_TWOCOLORTEX);
    gfx_set_combine_mode_arg(_G_CC_SPARSEST);
    gfx_set_combine_mode_arg(G_CC_TEMPLERP);
    gfx_set_combine_mode_arg(G_CC_TRILERP);
    gfx_set_combine_mode_arg(G_CC_INTERFERENCE);
    gfx_set_combine_mode_arg(G_CC_1CYUV2RGB);
    gfx_set_combine_mode_arg(G_CC_YUV2RGB);
    gfx_set_combine_mode_arg(G_CC_PASS2);
    gfx_set_combine_mode_arg(G_CC_MODULATEI2);
    gfx_set_combine_mode_arg(G_CC_MODULATEIA2);
    gfx_set_combine_mode_arg(G_CC_MODULATERGB2);
    gfx_set_combine_mode_arg(G_CC_MODULATERGBA2);
    gfx_set_combine_mode_arg(G_CC_MODULATEI_PRIM2);
    gfx_set_combine_mode_arg(G_CC_MODULATEIA_PRIM2);
    gfx_set_combine_mode_arg(G_CC_MODULATERGB_PRIM2);
    gfx_set_combine_mode_arg(G_CC_MODULATERGBA_PRIM2);
    gfx_set_combine_mode_arg(G_CC_DECALRGB2);
    gfx_set_combine_mode_arg(G_CC_BLENDI2);
    gfx_set_combine_mode_arg(G_CC_BLENDIA2);
    gfx_set_combine_mode_arg(G_CC_CHROMA_KEY2);
    gfx_set_combine_mode_arg(G_CC_HILITERGB2);
    gfx_set_combine_mode_arg(G_CC_HILITERGBA2);
    gfx_set_combine_mode_arg(G_CC_HILITERGBDECALA2);
    gfx_set_combine_mode_arg(G_CC_HILITERGBPASSA2);
    PrintError("  ERROR: Unknown gfx gsDPSetCombineMode arg: %s", _Arg.begin());
    return "";
}

static Array<s64> ParseGfxSetCombineMode(GfxData* aGfxData, DataNode<Gfx>* aNode, u64* pTokenIndex) {
    String _Buffer = ConvertSetCombineModeArgToString(aGfxData, aNode->mTokens[(*pTokenIndex)++]);
    Array<s64> _Args;
    String _Token;
    for (u64 i = 0, n = _Buffer.Length(); i <= n; ++i) {
        if (i == n || _Buffer[i] == ' ' || _Buffer[i] == '\t' || _Buffer[i] == ',') {
            if (_Token.Length() != 0) {
                String _Arg("%s%s", (_Args.Count() < 4 ? "G_CCMUX_" : "G_ACMUX_"), _Token.begin());
                _Args.Add(ParseGfxSymbolArg(aGfxData, aNode, NULL, _Arg.begin()));
                _Token.Clear();
            }
        } else {
            _Token.Add(_Buffer[i]);
        }
    }
    if (_Args.Count() < 8) {
        PrintError("  ERROR: gsDPSetCombineMode %s: Not enough arguments", _Buffer.begin());
    }
    return _Args;
}

static void UpdateTextureInfo(GfxData* aGfxData, s64 *aTexPtr, s32 aFormat, s32 aSize, s32 aWidth, s32 aHeight) {

    // Update current texture pointers
    if (aTexPtr && (*aTexPtr)) {
        aGfxData->mGfxContext.mCurrentPalette = aGfxData->mGfxContext.mCurrentTexture;
        aGfxData->mGfxContext.mCurrentTexture = (DataNode<TexData>*) (*aTexPtr);
    }

    // Update texture info if not loaded from a file
    if (aGfxData->mGfxContext.mCurrentTexture && aGfxData->mGfxContext.mCurrentTexture->mData && aGfxData->mGfxContext.mCurrentTexture->mData->mPngData.Empty()) {
        if (aFormat != -1) aGfxData->mGfxContext.mCurrentTexture->mData->mRawFormat = aFormat;
        if (aSize   != -1) aGfxData->mGfxContext.mCurrentTexture->mData->mRawSize   = aSize;
        if (aWidth  != -1) aGfxData->mGfxContext.mCurrentTexture->mData->mRawWidth  = aWidth;
        if (aHeight != -1) aGfxData->mGfxContext.mCurrentTexture->mData->mRawHeight = aHeight;
    }
}

static void ParseGfxSymbol(GfxData* aGfxData, DataNode<Gfx>* aNode, Gfx*& aHead, u64& aTokenIndex) {
    const String& _Symbol = aNode->mTokens[aTokenIndex++];

    // Simple symbols
    gfx_symbol_0(gsDPFullSync);
    gfx_symbol_0(gsDPTileSync);
    gfx_symbol_0(gsDPPipeSync);
    gfx_symbol_0(gsDPLoadSync);
    gfx_symbol_0(gsDPNoOp);
    gfx_symbol_1(gsDPNoOpTag, false);
    gfx_symbol_1(gsDPSetCycleType, false);
    gfx_symbol_2(gsSPLight, true);
    gfx_symbol_3(gsSPVertex, true);
    gfx_symbol_4(gsSP1Triangle);
    gfx_symbol_8(gsSP2Triangles);
    gfx_symbol_1(gsSPNumLights, false);
    gfx_symbol_1(gsDPSetDepthSource, false);
    gfx_symbol_1(gsDPSetTextureLUT, false);
    gfx_symbol_2(gsDPLoadTLUTCmd, false);
    gfx_symbol_5(gsDPLoadBlock);
    gfx_symbol_2(gsDPSetRenderMode, false);
    gfx_symbol_2(gsSPGeometryMode, false);
    gfx_symbol_6(gsDPSetPrimColor);
    gfx_symbol_4(gsDPSetEnvColor);
    gfx_symbol_4(gsDPSetFogColor);
    gfx_symbol_2(gsSPFogPosition, false);
    gfx_symbol_1(gsDPSetAlphaCompare, false);

    // Special symbols
    if (_Symbol == "gsSPTexture") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        gSPTexture(aHead++, _Arg0, _Arg1, _Arg2, _Arg3, _Arg4);
        return;
    }
    if (_Symbol == "gsSPSetGeometryMode") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        gSPSetGeometryMode(aHead++, _Arg0);
        return;
    }
    if (_Symbol == "gsSPClearGeometryMode") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        gSPClearGeometryMode(aHead++, _Arg0);
        return;
    }
    if (_Symbol == "gsSPDisplayList") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        aGfxData->mPointerList.Add(aHead);
        gSPDisplayList(aHead++, _Arg0);
        return;
    }
    if (_Symbol == "gsSPBranchList") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        aGfxData->mPointerList.Add(aHead);
        gSPBranchList(aHead++, _Arg0);
        return;
    }
    if (_Symbol == "gsSPEndDisplayList") {
        gSPEndDisplayList(aHead++);

        // Convert raw texture to PNG if all raw members are set
        if (aGfxData->mGfxContext.mCurrentTexture                    != NULL &&
            aGfxData->mGfxContext.mCurrentTexture->mData             != NULL &&
            aGfxData->mGfxContext.mCurrentTexture->mData->mPngData.Empty()   &&
            aGfxData->mGfxContext.mCurrentTexture->mData->mRawFormat != -1   &&
            aGfxData->mGfxContext.mCurrentTexture->mData->mRawSize   != -1   &&
            aGfxData->mGfxContext.mCurrentTexture->mData->mRawWidth  != -1   &&
            aGfxData->mGfxContext.mCurrentTexture->mData->mRawHeight != -1) {
            ConvertTextureDataToPng(aGfxData, aGfxData->mGfxContext.mCurrentTexture->mData);
        }

        // End the display list parsing after hitting gsSPEndDisplayList
        aTokenIndex = 0x7FFFFFFF;
        return;
    }

    // Complex symbols
    if (_Symbol == "gsSPSetLights1") {
        Lights1 *_Light = (Lights1 *) ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        gSPNumLights(aHead++, NUMLIGHTS_1);
        aGfxData->mPointerList.Add(aHead);
        gSPLight(aHead++, &_Light->l[0], 1);
        aGfxData->mPointerList.Add(aHead);
        gSPLight(aHead++, &_Light->a, 2);
        return;
    }
    if (_Symbol == "gsDPSetCombineMode") {
        Array<s64> _Args0 = ParseGfxSetCombineMode(aGfxData, aNode, &aTokenIndex);
        Array<s64> _Args1 = ParseGfxSetCombineMode(aGfxData, aNode, &aTokenIndex);
        Gfx _Gfx = {{
            _SHIFTL(G_SETCOMBINE, 24, 8) | _SHIFTL(GCCc0w0(_Args0[0x0], _Args0[0x2], _Args0[0x4], _Args0[0x6]) | GCCc1w0(_Args1[0x0], _Args1[0x2]), 0, 24),
            (u32) (GCCc0w1(_Args0[0x1], _Args0[0x3], _Args0[0x5], _Args0[0x7]) | GCCc1w1(_Args1[0x1], _Args1[0x4], _Args1[0x6], _Args1[0x3], _Args1[0x5], _Args1[0x7]))
        }};
        *(aHead++) = _Gfx;
        return;
    }
    if (_Symbol == "gsDPSetCombineLERP") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_CCMUX_");
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_CCMUX_");
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_CCMUX_");
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_CCMUX_");
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_ACMUX_");
        s64 _Arg5 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_ACMUX_");
        s64 _Arg6 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_ACMUX_");
        s64 _Arg7 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_ACMUX_");
        s64 _Arg8 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_CCMUX_");
        s64 _Arg9 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_CCMUX_");
        s64 _ArgA = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_CCMUX_");
        s64 _ArgB = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_CCMUX_");
        s64 _ArgC = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_ACMUX_");
        s64 _ArgD = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_ACMUX_");
        s64 _ArgE = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_ACMUX_");
        s64 _ArgF = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "G_ACMUX_");
        Gfx _Gfx = {{
            _SHIFTL(G_SETCOMBINE, 24, 8) | _SHIFTL(GCCc0w0(_Arg0, _Arg2, _Arg4, _Arg6) | GCCc1w0(_Arg8, _ArgA), 0, 24),
            (u32) (GCCc0w1(_Arg1, _Arg3, _Arg5, _Arg7) | GCCc1w1(_Arg9, _ArgC, _ArgE, _ArgB, _ArgD, _ArgF))
        }};
        *(aHead++) = _Gfx;
        return;
    }

    // TexData symbols
    if (_Symbol == "gsDPSetTextureImage") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        UpdateTextureInfo(aGfxData, &_Arg3, (s32) _Arg0, (s32) _Arg1, -1, -1);
        aGfxData->mPointerList.Add(aHead);
        gDPSetTextureImage(aHead++, _Arg0, _Arg1, _Arg2, _Arg3);
        return;
    }
    if (_Symbol == "gsDPSetTile") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg5 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg6 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg7 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg8 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg9 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _ArgA = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _ArgB = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        UpdateTextureInfo(aGfxData, NULL, (s32) _Arg0, (s32) _Arg1, -1, -1);
        gDPSetTile(aHead++, _Arg0, _Arg1, _Arg2, _Arg3, _Arg4, _Arg5, _Arg6, _Arg7, _Arg8, _Arg9, _ArgA, _ArgB);
        return;
    }
    if (_Symbol == "gsDPLoadTile") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        UpdateTextureInfo(aGfxData, NULL, -1, -1, (s32) (_Arg3 >> G_TEXTURE_IMAGE_FRAC) + 1, (s32) (_Arg4 >> G_TEXTURE_IMAGE_FRAC) + 1);
        gDPLoadTile(aHead++, _Arg0, _Arg1, _Arg2, _Arg3, _Arg4);
        return;
    }
    if (_Symbol == "gsDPSetTileSize") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        UpdateTextureInfo(aGfxData, NULL, -1, -1, (s32) (_Arg3 >> G_TEXTURE_IMAGE_FRAC) + 1, (s32) (_Arg4 >> G_TEXTURE_IMAGE_FRAC) + 1);
        gDPSetTileSize(aHead++, _Arg0, _Arg1, _Arg2, _Arg3, _Arg4);
        return;
    }
    if (_Symbol == "gsDPLoadTextureBlock") {
        s64 _Arg0 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg1 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        gfx_arg_with_suffix(arg2_0, _LOAD_BLOCK);
        gfx_arg_with_suffix(arg2_1, _INCR);
        gfx_arg_with_suffix(arg2_2, _SHIFT);
        gfx_arg_with_suffix(arg2_3, _BYTES);
        gfx_arg_with_suffix(arg2_4, _LINE_BYTES);
        s64 _Arg2 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg3 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg4 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg5 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg6 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg7 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg8 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _Arg9 = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _ArgA = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        s64 _ArgB = ParseGfxSymbolArg(aGfxData, aNode, &aTokenIndex, "");
        UpdateTextureInfo(aGfxData, &_Arg0, (s32) _Arg1, (s32) _Arg2, (s32) _Arg3, (s32) _Arg4);

        aGfxData->mPointerList.Add(aHead);
        gDPSetTextureImage(aHead++, _Arg1, arg2_0, 1, _Arg0);
        gDPSetTile(aHead++, _Arg1, arg2_0, 0, 0, G_TX_LOADTILE, 0, _Arg7, _Arg9, _ArgB, _Arg6, _Arg8, _ArgA);
        gDPLoadSync(aHead++);
        gDPLoadBlock(aHead++, G_TX_LOADTILE, 0, 0, (((_Arg3) * (_Arg4) + arg2_1) >> arg2_2) - 1, CALC_DXT(_Arg3, arg2_3));
        gDPPipeSync(aHead++);
        gDPSetTile(aHead++, _Arg1, _Arg2, ((((_Arg3) * arg2_4) + 7) >> 3), 0, G_TX_RENDERTILE, _Arg5, _Arg7, _Arg9, _ArgB, _Arg6, _Arg8, _ArgA);
        gDPSetTileSize(aHead++, G_TX_RENDERTILE, 0, 0, ((_Arg3) - 1) << G_TEXTURE_IMAGE_FRAC, ((_Arg4) - 1) << G_TEXTURE_IMAGE_FRAC);
        return;
    }

    // Unknown
    PrintError("  ERROR: Unknown gfx symbol: %s", _Symbol.begin());
}

static DataNode<Gfx>* ParseDisplayListData(GfxData* aGfxData, DataNode<Gfx>* aNode) {
    if (aNode->mData) return aNode;

    // Display list data
    aNode->mData = New<Gfx>(aNode->mTokens.Count() * DISPLAY_LIST_SIZE_PER_TOKEN);
    Gfx* _Head = aNode->mData;
    for (u64 _TokenIndex = 0; _TokenIndex < aNode->mTokens.Count();) { // Don't increment _TokenIndex here!
        ParseGfxSymbol(aGfxData, aNode, _Head, _TokenIndex);
    }
    aNode->mSize = (u32) (_Head - aNode->mData);
    aNode->mLoadIndex = aGfxData->mLoadIndex++;
    return aNode;
}

//
// Geo layouts
//

#define geo_constant(x) if (_Arg == #x) { return (s64) (x); }
static s64 ParseGeoSymbolArg(GfxData* aGfxData, DataNode<GeoLayout>* aNode, u64& aTokenIndex) {
    const String& _Arg = aNode->mTokens[aTokenIndex++];

    // Geo functions
    void *_GeoFunctionPtr = DynOS_Geo_GetFunctionPointerFromName(_Arg);
    if (_GeoFunctionPtr != NULL) {
        return (s64) _GeoFunctionPtr;
    }

    // Layer constants
    geo_constant(LAYER_FORCE);
    geo_constant(LAYER_OPAQUE);
    geo_constant(LAYER_OPAQUE_DECAL);
    geo_constant(LAYER_OPAQUE_INTER);
    geo_constant(LAYER_ALPHA);
    geo_constant(LAYER_TRANSPARENT);
    geo_constant(LAYER_TRANSPARENT_DECAL);
    geo_constant(LAYER_TRANSPARENT_INTER);

    // Background constants
    geo_constant(BACKGROUND_OCEAN_SKY);
    geo_constant(BACKGROUND_FLAMING_SKY);
    geo_constant(BACKGROUND_UNDERWATER_CITY);
    geo_constant(BACKGROUND_BELOW_CLOUDS);
    geo_constant(BACKGROUND_SNOW_MOUNTAINS);
    geo_constant(BACKGROUND_DESERT);
    geo_constant(BACKGROUND_HAUNTED);
    geo_constant(BACKGROUND_GREEN_SKY);
    geo_constant(BACKGROUND_ABOVE_CLOUDS);
    geo_constant(BACKGROUND_PURPLE_SKY);

    // Shadow constants
    geo_constant(SHADOW_CIRCLE_9_VERTS);
    geo_constant(SHADOW_CIRCLE_4_VERTS);
    geo_constant(SHADOW_CIRCLE_4_VERTS_FLAT_UNUSED);
    geo_constant(SHADOW_SQUARE_PERMANENT);
    geo_constant(SHADOW_SQUARE_SCALABLE);
    geo_constant(SHADOW_SQUARE_TOGGLABLE);
    geo_constant(SHADOW_RECTANGLE_HARDCODED_OFFSET);
    geo_constant(SHADOW_CIRCLE_PLAYER);

    // Other constants
    geo_constant(NULL);
    geo_constant(SCREEN_WIDTH);
    geo_constant(SCREEN_HEIGHT);
    geo_constant(SCREEN_WIDTH/2);
    geo_constant(SCREEN_HEIGHT/2);

    // Display lists
    for (auto& _Node : aGfxData->mDisplayLists) {
        if (_Arg == _Node->mName) {
            return (s64) ParseDisplayListData(aGfxData, _Node);
        }
    }

    // Geo layouts
    for (auto& _Node : aGfxData->mGeoLayouts) {
        if (_Arg == _Node->mName) {
            return (s64) ParseGeoLayoutData(aGfxData, _Node, false)->mData;
        }
    }

    // Integers
    s32 x;
    if ((_Arg[1] == 'x' && sscanf(_Arg.begin(), "%x", &x) == 1) || (sscanf(_Arg.begin(), "%d", &x) == 1)) {
        return (s64) x;
    }

    // Unknown
    PrintError("  ERROR: Unknown geo arg: %s", _Arg.begin());
    return 0;
}

#define geo_symbol_0(symb)                                                                             \
    if (_Symbol == #symb) {                                                                       \
        GeoLayout _Gl[] = { symb() };                                                                   \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

#define geo_symbol_1(symb, n)                                                                          \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        if (n != 0) { aGfxData->mPointerList.Add(aHead + n); }                            \
        GeoLayout _Gl[] = { symb(_Arg0) };                                                               \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

#define geo_symbol_2(symb, n)                                                                          \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg1 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        if (n != 0) { aGfxData->mPointerList.Add(aHead + n); }                            \
        GeoLayout _Gl[] = { symb(_Arg0, _Arg1) };                                                         \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

#define geo_symbol_3(symb, n)                                                                          \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg1 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg2 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        if (n != 0) { aGfxData->mPointerList.Add(aHead + n); }                            \
        GeoLayout _Gl[] = { symb(_Arg0, _Arg1, _Arg2) };                                                   \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

#define geo_symbol_4(symb, n)                                                                          \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg1 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg2 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg3 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        if (n != 0) { aGfxData->mPointerList.Add(aHead + n); }                            \
        GeoLayout _Gl[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3) };                                             \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

#define geo_symbol_5(symb, n)                                                                          \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg1 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg2 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg3 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg4 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        if (n != 0) { aGfxData->mPointerList.Add(aHead + n); }                            \
        GeoLayout _Gl[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4) };                                       \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

#define geo_symbol_6(symb, n)                                                                          \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg1 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg2 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg3 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg4 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg5 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        if (n != 0) { aGfxData->mPointerList.Add(aHead + n); }                            \
        GeoLayout _Gl[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4, _Arg5) };                                 \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

#define geo_symbol_7(symb, n)                                                                          \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg1 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg2 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg3 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg4 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg5 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg6 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        if (n != 0) { aGfxData->mPointerList.Add(aHead + n); }                            \
        GeoLayout _Gl[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4, _Arg5, _Arg6) };                           \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

#define geo_symbol_8(symb, n)                                                                          \
    if (_Symbol == #symb) {                                                                       \
        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg1 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg2 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg3 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg4 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg5 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg6 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        s64 _Arg7 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);                                    \
        if (n != 0) { aGfxData->mPointerList.Add(aHead + n); }                            \
        GeoLayout _Gl[] = { symb(_Arg0, _Arg1, _Arg2, _Arg3, _Arg4, _Arg5, _Arg6, _Arg7) };                     \
        memcpy(aHead, _Gl, sizeof(_Gl));                                                                 \
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));                                                         \
        return;                                                                                        \
    }

static void ParseGeoSymbol(GfxData* aGfxData, DataNode<GeoLayout>* aNode, GeoLayout*& aHead, u64& aTokenIndex, Array<u64>& aSwitchNodes) {
    const String& _Symbol = aNode->mTokens[aTokenIndex++];

    // Restore context after each command if inside a switch
    if (!aSwitchNodes.Empty() && !aGfxData->mGeoNodeStack.Empty()) {
        aGfxData->mGfxContext = aGfxData->mGeoNodeStack[aGfxData->mGeoNodeStack.Count() - 1];
    }

    geo_symbol_1(GEO_BRANCH_AND_LINK, 1);
    geo_symbol_0(GEO_END);
    geo_symbol_2(GEO_BRANCH, 1);
    geo_symbol_0(GEO_RETURN);
    geo_symbol_5(GEO_NODE_SCREEN_AREA, 0);
    geo_symbol_1(GEO_NODE_ORTHO, 0);
    geo_symbol_3(GEO_CAMERA_FRUSTUM, 0);
    geo_symbol_4(GEO_CAMERA_FRUSTUM_WITH_FUNC, 2);
    geo_symbol_0(GEO_NODE_START);
    geo_symbol_1(GEO_ZBUFFER, 0);
    geo_symbol_2(GEO_RENDER_RANGE, 0);
    geo_symbol_8(GEO_CAMERA, 4);
    geo_symbol_7(GEO_TRANSLATE_ROTATE, 0);
    geo_symbol_8(GEO_TRANSLATE_ROTATE_WITH_DL, 4);
    geo_symbol_4(GEO_TRANSLATE, 0);
    geo_symbol_5(GEO_TRANSLATE_WITH_DL, 2);
    geo_symbol_4(GEO_ROTATE, 0);
    geo_symbol_5(GEO_ROTATE_WITH_DL, 2);
    geo_symbol_2(GEO_ROTATE_Y, 0);
    geo_symbol_3(GEO_ROTATE_Y_WITH_DL, 1);
    geo_symbol_4(GEO_TRANSLATE_NODE, 0);
    geo_symbol_5(GEO_TRANSLATE_NODE_WITH_DL, 2);
    geo_symbol_4(GEO_ROTATION_NODE, 0);
    geo_symbol_5(GEO_ROTATION_NODE_WITH_DL, 2);
    geo_symbol_5(GEO_ANIMATED_PART, 2);
    geo_symbol_4(GEO_BILLBOARD_WITH_PARAMS, 0);
    geo_symbol_5(GEO_BILLBOARD_WITH_PARAMS_AND_DL, 2);
    geo_symbol_0(GEO_BILLBOARD);
    geo_symbol_2(GEO_DISPLAY_LIST, 1);
    geo_symbol_3(GEO_SHADOW, 0);
    geo_symbol_0(GEO_RENDER_OBJ);
    geo_symbol_2(GEO_ASM, 1);
    geo_symbol_2(GEO_BACKGROUND, 1);
    geo_symbol_1(GEO_BACKGROUND_COLOR, 0);
    geo_symbol_0(GEO_NOP_1A);
    geo_symbol_5(GEO_HELD_OBJECT, 2);
    geo_symbol_2(GEO_SCALE, 0);
    geo_symbol_3(GEO_SCALE_WITH_DL, 2);
    geo_symbol_0(GEO_NOP_1E);
    geo_symbol_0(GEO_NOP_1F);
    geo_symbol_1(GEO_CULLING_RADIUS, 0);

    // Switch node
    if (_Symbol == "GEO_SWITCH_CASE") {

        // Start a switch
        aSwitchNodes.Add(0);

        s64 _Arg0 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);
        s64 _Arg1 = ParseGeoSymbolArg(aGfxData, aNode, aTokenIndex);
        aGfxData->mPointerList.Add(aHead + 1);
        GeoLayout _Gl[] = { GEO_SWITCH_CASE(_Arg0, _Arg1) };
        memcpy(aHead, _Gl, sizeof(_Gl));
        aHead += (sizeof(_Gl) / sizeof(_Gl[0]));
        return;
    }

    // Open node
    if (_Symbol == "GEO_OPEN_NODE") {

        // We're inside a switch
        if (!aSwitchNodes.Empty()) {
            aSwitchNodes[aSwitchNodes.Count() - 1]++;
        }

        // Push context
        aGfxData->mGeoNodeStack.Add(aGfxData->mGfxContext);

        *(aHead++) = GEO_OPEN_NODE();
        return;
    }

    // Close node
    if (_Symbol == "GEO_CLOSE_NODE") {

        // Are we still inside a switch?
        if (!aSwitchNodes.Empty()) {
            aSwitchNodes[aSwitchNodes.Count() - 1]--;

            // We're not anymore
            if (aSwitchNodes[aSwitchNodes.Count() - 1] == 0) {
                aSwitchNodes.Pop();
            }
        }

        // Pop context
        if (!aGfxData->mGeoNodeStack.Empty()) {
            aGfxData->mGfxContext = aGfxData->mGeoNodeStack[aGfxData->mGeoNodeStack.Count() - 1];
            aGfxData->mGeoNodeStack.Pop();
        }

        *(aHead++) = GEO_CLOSE_NODE();
        return;
    }

    // Unknown
    PrintError("  ERROR: Unknown geo symbol: %s", _Symbol.begin());
}

static DataNode<GeoLayout>* ParseGeoLayoutData(GfxData* aGfxData, DataNode<GeoLayout>* aNode, bool aDisplayPercent) {
    if (aNode->mData) return aNode;

    // Geo layout data
    aNode->mData = New<GeoLayout>(aNode->mTokens.Count() * GEO_LAYOUT_SIZE_PER_TOKEN);
    GeoLayout* _Head = aNode->mData;
    Array<u64> _SwitchNodes;
    for (u64 _TokenIndex = 0; _TokenIndex < aNode->mTokens.Count();) { // Don't increment _TokenIndex here!
        ParseGeoSymbol(aGfxData, aNode, _Head, _TokenIndex, _SwitchNodes);
        if (aDisplayPercent && aGfxData->mErrorCount == 0) { PrintNoNewLine("%3d%%\b\b\b\b", (s32) (_TokenIndex * 100) / aNode->mTokens.Count()); }
    }
    if (aDisplayPercent && aGfxData->mErrorCount == 0) { Print("100%%"); }
    aNode->mSize = (u32)(_Head - aNode->mData);
    aNode->mLoadIndex = aGfxData->mLoadIndex++;
    return aNode;
}

//
// Animation files
//

static void ScanAnimationDataFile(GfxData *aGfxData, const SysPath &aFilename) {
    FILE *_File = fopen(aFilename.c_str(), "rb");
    if (!_File) {
        PrintError("  ERROR: Unable to open file \"%s\"", aFilename.c_str());
    }

    // Load file into a buffer while removing all comments
    char *_FileBuffer = LoadFileBuffer(_File, NULL);
    fclose(_File);

    // Parse animation data
    u8 _DataType = DATA_TYPE_NONE;
    String _DataName;
    bool _IsData = false;
    Array<String> _Data;
    Array<String> _Tokens = Split(_FileBuffer, " []()=&,;\t\r\n\b");
    for (const auto &_Token : _Tokens) {

        // Data type
        if (_DataType == DATA_TYPE_NONE) {
            if (_Token == "s16" || _Token == "u16" || _Token == "s16") {
                _DataType = DATA_TYPE_ANIMATION_VALUE;
            } else if (_Token == "Animation") {
                _DataType = DATA_TYPE_ANIMATION;
            }
        }

        // Data name
        else if (_DataName.Empty()) {
            _DataName = _Token;
            if (_DataType == DATA_TYPE_ANIMATION_VALUE && (_DataName.Find("index") != -1 || _DataName.Find("indices") != -1)) {
                _DataType = DATA_TYPE_ANIMATION_INDEX;
            }
        }

        // Is data?
        else if (!_IsData) {
            if (_Token == "{") {
                _IsData = true;
            }
        }

        // Data
        else {
            if (_Token == "}") {
                switch (_DataType) {
                    case DATA_TYPE_ANIMATION_VALUE: {
                        AnimBuffer<s16> *_AnimValues = New<AnimBuffer<s16>>();
                        _AnimValues->first = _DataName;
                        for (const auto &_Value : _Data) {
                            _AnimValues->second.Add(_Value.ParseInt());
                        }
                        aGfxData->mAnimValues.Add(_AnimValues);
                    } break;

                    case DATA_TYPE_ANIMATION_INDEX: {
                        AnimBuffer<u16> *_AnimIndices = New<AnimBuffer<u16>>();
                        _AnimIndices->first = _DataName;
                        for (const auto &_Index : _Data) {
                            _AnimIndices->second.Add(_Index.ParseInt());
                        }
                        aGfxData->mAnimIndices.Add(_AnimIndices);
                    } break;

                    case DATA_TYPE_ANIMATION: {
                        if (_Data.Count() < 10) {
                            PrintError("  ERROR: %s: Not enough data", _DataName.begin());
                            break;
                        }

                        DataNode<AnimData> *_Node = New<DataNode<AnimData>>();
                        _Node->mName = _DataName;
                        _Node->mData = New<AnimData>();
                        _Node->mData->mFlags = (s16) _Data[0].ParseInt();
                        _Node->mData->mUnk02 = (s16) _Data[1].ParseInt();
                        _Node->mData->mUnk04 = (s16) _Data[2].ParseInt();
                        _Node->mData->mUnk06 = (s16) _Data[3].ParseInt();
                        _Node->mData->mUnk08 = (s16) _Data[4].ParseInt();
                        _Node->mData->mUnk0A.first = _Data[6]; // 5 is "ANIMINDEX_NUMPARTS"
                        _Node->mData->mValues.first = _Data[7];
                        _Node->mData->mIndex.first = _Data[8];
                        _Node->mData->mLength = (u32) _Data[9].ParseInt();
                        aGfxData->mAnimations.Add(_Node);
                    } break;
                }
                _DataType = DATA_TYPE_NONE;
                _DataName.Clear();
                _IsData = false;
                _Data.Clear();
            } else {
                _Data.Add(_Token);
            }
        }
    }
    Delete(_FileBuffer);
}

static void ScanAnimationTableFile(GfxData *aGfxData, const SysPath &aFilename) {
    FILE *_File = fopen(aFilename.c_str(), "rb");
    if (!_File) {
        PrintError("  ERROR: Unable to open file \"%s\"", aFilename.c_str());
    }

    // Load file into a buffer while removing all comments
    char *_FileBuffer = LoadFileBuffer(_File, NULL);
    fclose(_File);

    // Retrieve animation names
    bool _IsAnimName = false;
    Array<String> _Tokens = Split(_FileBuffer, " =&,;\t\r\n\b");
    for (const auto &_Token : _Tokens) {
        if (_Token == "{") {
            _IsAnimName = true;
        } else if (_Token == "}") {
            _IsAnimName = false;
        } else if (_IsAnimName) {
            aGfxData->mAnimationTable.Add({ _Token, NULL });
        }
    }
    Delete(_FileBuffer);
}

static void ScanAnimationFolder(GfxData *aGfxData, const SysPath &aAnimsFolder) {
    DIR *_AnimsDir = opendir(aAnimsFolder.c_str());
    if (!_AnimsDir) return;

    struct dirent *_AnimsEnt = NULL;
    while ((_AnimsEnt = readdir(_AnimsDir)) != NULL) {

        // Skip
        if (SysPath(_AnimsEnt->d_name) == ".") continue;
        if (SysPath(_AnimsEnt->d_name) == "..") continue;
        if (SysPath(_AnimsEnt->d_name) == "data.inc.c") continue;

        // Animation file
        SysPath _AnimsFilename = fstring("%s/%s", aAnimsFolder.c_str(), _AnimsEnt->d_name);
        if (fs_sys_file_exists(_AnimsFilename.c_str())) {

            // Table file
            if (SysPath(_AnimsEnt->d_name) == "table.inc.c") {
                ScanAnimationTableFile(aGfxData, _AnimsFilename);
            }

            // Data file
            else {
                ScanAnimationDataFile(aGfxData, _AnimsFilename);
            }
        }
    }
    closedir(_AnimsDir);
}

//
// Read & Generate
//

// Free data pointers, but keep nodes and tokens intact
// Delete nodes generated from GfxDynCmds
template <typename T>
static void ClearGfxDataNodes(DataNodes<T> &aDataNodes) {
    for (s32 i = aDataNodes.Count(); i != 0; --i) {
        Delete(aDataNodes[i - 1]->mData);
    }
}

static DataNode<GeoLayout> *GetGeoLayout(GfxData *aGfxData, const String& aGeoRoot) {
    for (DataNode<GeoLayout> *_Node : aGfxData->mGeoLayouts) {
        if (_Node->mName == aGeoRoot) {
            return _Node;
        }
    }
    return NULL;
}

static String GetActorFolder(const Array<Pair<u64, String>> &aActorsFolders, u64 aModelIdentifier) {
    for (const auto &_Pair : aActorsFolders) {
        if (_Pair.first == aModelIdentifier) {
            return _Pair.second;
        }
    }
    return String();
}

void DynOS_Gfx_GeneratePack(const SysPath &aPackFolder) {
    Print("---------- Pack folder: \"%s\" ----------", aPackFolder.c_str());
    Array<Pair<u64, String>> _ActorsFolders;
    GfxData *_GfxData = New<GfxData>();

    // Read all the model.inc.c files and geo.inc.c files from the subfolders of the pack folder
    // Animations are processed separately
    DIR *aPackDir = opendir(aPackFolder.c_str());
    if (aPackDir) {
        struct dirent *_PackEnt = NULL;
        while ((_PackEnt = readdir(aPackDir)) != NULL) {

            // Skip . and ..
            if (SysPath(_PackEnt->d_name) == ".") continue;
            if (SysPath(_PackEnt->d_name) == "..") continue;

            // For each subfolder, read tokens from model.inc.c and geo.inc.c
            SysPath _Folder = fstring("%s/%s", aPackFolder.c_str(), _PackEnt->d_name);
            if (fs_sys_dir_exists(_Folder.c_str())) {
                _GfxData->mModelIdentifier = 0;
                ScanModelFile(_GfxData, fstring("%s/model.inc.c", _Folder.c_str()));
                ScanModelFile(_GfxData, fstring("%s/geo.inc.c", _Folder.c_str()));
                if (_GfxData->mModelIdentifier != 0) {
                    _ActorsFolders.Add({ _GfxData->mModelIdentifier, String(_PackEnt->d_name) });
                }
            }
        }
        closedir(aPackDir);
    }

    // Generate a binary file for each actor found in the GfxData
    for (s32 i = 0; i != DynOS_Geo_GetActorCount(); ++i) {
        String _GeoRootName = DynOS_Geo_GetActorName(i);
        DataNode<GeoLayout> *_GeoRoot = GetGeoLayout(_GfxData, _GeoRootName);
        if (_GeoRoot != NULL) {

            // If there is an existing binary file for this layout, skip and go to the next actor
            SysPath _BinFilename = fstring("%s/%s.bin", aPackFolder.c_str(), _GeoRootName.begin());
            if (fs_sys_file_exists(_BinFilename.c_str())) {
                continue;
            }

            // Init
            _GfxData->mLoadIndex                  = 0;
            _GfxData->mErrorCount                 = 0;
            _GfxData->mModelIdentifier            = _GeoRoot->mModelIdentifier;
            _GfxData->mPackFolder                 = aPackFolder;
            _GfxData->mPointerList                = { NULL }; // The NULL pointer is needed, so we add it here
            _GfxData->mGfxContext.mCurrentTexture = NULL;
            _GfxData->mGfxContext.mCurrentPalette = NULL;
            _GfxData->mGeoNodeStack.Clear();

            // Parse data
            PrintNoNewLine("%s.bin: Model identifier: %X - Processing... ", _GeoRootName.begin(), _GfxData->mModelIdentifier);
            ParseGeoLayoutData(_GfxData, _GeoRoot, true);

            // Init animation data
            for (auto &_AnimBuffer : _GfxData->mAnimValues) Delete(_AnimBuffer);
            for (auto &_AnimBuffer : _GfxData->mAnimIndices) Delete(_AnimBuffer);
            for (auto &_AnimNode : _GfxData->mAnimations) Delete(_AnimNode);
            _GfxData->mAnimValues.Clear();
            _GfxData->mAnimIndices.Clear();
            _GfxData->mAnimations.Clear();
            _GfxData->mAnimationTable.Clear();

            // Scan anims folder for animation data
            String _ActorFolder = GetActorFolder(_ActorsFolders, _GfxData->mModelIdentifier);
            SysPath _AnimsFolder = fstring("%s/%s/anims", aPackFolder.c_str(), _ActorFolder.begin());
            ScanAnimationFolder(_GfxData, _AnimsFolder);

            // Create table for mario_geo animations or luigi_geo animations
            if ((_GeoRootName == "mario_geo" || _GeoRootName == "luigi_geo") && !_GfxData->mAnimations.Empty()) {
                _GfxData->mAnimationTable.Resize(256);
                for (s32 i = 0; i != 256; ++i) {
                    String _AnimName("anim_%02X", i);
                    if (_GfxData->mAnimations.FindIf([&_AnimName](const DataNode<AnimData> *aNode) { return aNode->mName == _AnimName; }) != -1) {
                        _GfxData->mAnimationTable[i] = { _AnimName, NULL };
                    } else {
                        _GfxData->mAnimationTable[i] = { "NULL", NULL };
                    }
                }
            }

            // Write if no error
            if (_GfxData->mErrorCount == 0) {
                DynOS_Gfx_WriteBinary(_BinFilename, _GfxData);
            } else {
                Print("  %u error(s): Unable to parse data", _GfxData->mErrorCount);
            }

            // Clear data pointers
            ClearGfxDataNodes(_GfxData->mLights);
            ClearGfxDataNodes(_GfxData->mTextures);
            ClearGfxDataNodes(_GfxData->mVertices);
            ClearGfxDataNodes(_GfxData->mDisplayLists);
            ClearGfxDataNodes(_GfxData->mGeoLayouts);
        }
    }

    DynOS_Gfx_Free(_GfxData);
}

#pragma GCC diagnostic pop
