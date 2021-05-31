#include "mod-model.h"

#include "moon/mod-engine/interfaces/file-entry.h"
#include "moon/mod-engine/interfaces/bit-module.h"
#include "types.h"
#include <string>
#include <vector>
#include <iostream>
#include <map>

using namespace std;

map<int, GraphNode*> loadedGraphNodes;

extern "C"{
void bind_graph_node(int modelId, struct GraphNode *graphNode){
    loadedGraphNodes[modelId] = graphNode;
    // cout << "Binding model with id " << modelId << endl;
}
struct GraphNode * get_graph_node(int modelId){
    if(loadedGraphNodes.find(modelId) == loadedGraphNodes.end()) return NULL;
    // cout << "Loading model with id " << modelId << endl;
    return loadedGraphNodes[modelId];
}
}