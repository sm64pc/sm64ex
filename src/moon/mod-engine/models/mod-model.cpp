#include "mod-model.h"

#include "moon/mod-engine/interfaces/file-entry.h"
#include "moon/mod-engine/interfaces/bit-module.h"
#include "moon/mod-engine/hooks/hook.h"
#include "types.h"
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "model_ids.h"

using namespace std;

map<int, GraphNode*> loadedGraphNodes;

namespace MoonInternal {
    void setupModelEngine(string state) {
        if(state == "Init"){}
    }
}

extern "C"{
void bind_graph_node(int modelId, struct GraphNode *graphNode){
    MoonInternal::bindHook(SAVE_GRAPH_NODE);
    MoonInternal::initBindHook(2,
        (struct HookParameter){.name = "modelId", .parameter = (void*) &modelId},
        (struct HookParameter){.name = "graphNode", .parameter = (void*) &graphNode}
    );
    MoonInternal::callBindHook(0);
    loadedGraphNodes[modelId] = graphNode;
}
struct GraphNode * get_graph_node(int modelId){
    MoonInternal::bindHook(LOAD_GRAPH_NODE);
    MoonInternal::initBindHook(1,
        (struct HookParameter){.name = "modelId", .parameter = (void*) &modelId}
    );
    MoonInternal::callBindHook(0);
    return loadedGraphNodes[modelId];
}
}