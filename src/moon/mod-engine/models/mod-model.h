#ifndef ModEngineModelModule
#define ModEngineModelModule

#include "types.h"

#ifdef __cplusplus
#include <string>

namespace Moon {
    GraphNode * GetGraphNode(int modelId);
    int GetGraphNodeID( GraphNode* graphNode );
}

namespace MoonInternal {
    void setupModelEngine( std::string state );
}

#else

void bind_graph_node(int modelId, struct GraphNode *graphNode);
struct GraphNode * get_graph_node(int modelId);

#endif
#endif