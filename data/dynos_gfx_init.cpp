#include "dynos.cpp.h"

Array<ActorGfx> &DynOS_Gfx_GetActorList() {
    static Array<ActorGfx> sActorGfxList;
    return sActorGfxList;
}

Array<PackData *> &DynOS_Gfx_GetPacks() {
    static Array<PackData *> sPacks;
    return sPacks;
}

Array<String> DynOS_Gfx_Init() {

    // Alloc and init the actors gfx list
    Array<ActorGfx> &pActorGfxList = DynOS_Gfx_GetActorList();
    pActorGfxList.Resize(DynOS_Geo_GetActorCount());
    for (s32 i = 0; i != DynOS_Geo_GetActorCount(); ++i) {
        pActorGfxList[i].mPackIndex = -1;
        pActorGfxList[i].mGfxData   = NULL;
        pActorGfxList[i].mGraphNode = (GraphNode *) DynOS_Geo_GetGraphNode(DynOS_Geo_GetActorLayout(i), false);
    }

    // Scan the DynOS packs folder
    Array<PackData *> &pDynosPacks = DynOS_Gfx_GetPacks();
    SysPath _DynosPacksFolder = fstring("%s/%s", DYNOS_EXE_FOLDER, DYNOS_PACKS_FOLDER);
    DIR *_DynosPacksDir = opendir(_DynosPacksFolder.c_str());
    if (_DynosPacksDir) {
        struct dirent *_DynosPacksEnt = NULL;
        while ((_DynosPacksEnt = readdir(_DynosPacksDir)) != NULL) {

            // Skip . and ..
            if (SysPath(_DynosPacksEnt->d_name) == ".") continue;
            if (SysPath(_DynosPacksEnt->d_name) == "..") continue;

            // If pack folder exists, add it to the pack list
            SysPath _PackFolder = fstring("%s/%s", _DynosPacksFolder.c_str(), _DynosPacksEnt->d_name);
            if (fs_sys_dir_exists(_PackFolder.c_str())) {
                PackData *_Pack = New<PackData>();

                // Scan folder for subfolders to convert into .bin files
                _Pack->mPath = _PackFolder;
                DynOS_Gfx_GeneratePack(_PackFolder);

                // Add pack to pack list
                pDynosPacks.Add(_Pack);
            }
        }
        closedir(_DynosPacksDir);
    }

    // Return a list of pack names
    Array<String> _PackNames;
    for (const auto& _Pack : pDynosPacks) {
        u64 _DirSep1 = _Pack->mPath.find_last_of('\\');
        u64 _DirSep2 = _Pack->mPath.find_last_of('/');
        if (_DirSep1++ == SysPath::npos) _DirSep1 = 0;
        if (_DirSep2++ == SysPath::npos) _DirSep2 = 0;
        SysPath _DirName = _Pack->mPath.substr(MAX(_DirSep1, _DirSep2));
        _PackNames.Add(_DirName.c_str());
    }
    return _PackNames;
}
