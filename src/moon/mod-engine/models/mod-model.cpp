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
#include <algorithm>

using namespace std;

map<int, GraphNode*> loadedGraphNodes;

namespace Moon {
    GraphNode * GetGraphNode(int modelId){
        MoonInternal::bindHook(LOAD_GRAPH_NODE);
        MoonInternal::initBindHook(1,
            (struct HookParameter){.name = "modelId", .parameter = (void*) &modelId}
        );
        GraphNode* graphNode = loadedGraphNodes[modelId];
        if(graphNode == NULL) return NULL;
        MoonInternal::callBindHook(1,
            (HookParameter){.name = "graphNode", .parameter = (void*) &graphNode}
        );
        return graphNode;
    }

    int GetGraphNodeID( GraphNode* graphNode ){
        auto findResult = std::find_if(std::begin(loadedGraphNodes), std::end(loadedGraphNodes), [&](const std::pair<int, GraphNode*> &pair) {
            if(pair.second == nullptr || graphNode == nullptr) return false;
            return pair.second->prev == graphNode;
        });
        if (findResult != std::end(loadedGraphNodes))
            return findResult->first;
        return -1;
    }

}
namespace MoonInternal {
    void setupModelEngine(string state) {
        if(state == "Init"){}
    }
}

extern "C"{
void bind_graph_node(int modelId, GraphNode *graphNode){
    MoonInternal::bindHook(SAVE_GRAPH_NODE);
    MoonInternal::initBindHook(2,
        (HookParameter){.name = "modelId", .parameter = (void*) &modelId},
        (HookParameter){.name = "graphNode", .parameter = (void*) &graphNode}
    );
    MoonInternal::callBindHook(0);
    loadedGraphNodes[modelId] = graphNode;
}
struct GraphNode * get_graph_node(int modelId){
    return Moon::GetGraphNode(modelId);
}
}