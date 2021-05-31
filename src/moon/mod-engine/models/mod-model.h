#ifndef ModEngineModelModule
#define ModEngineModelModule

#ifdef __cplusplus
#include <string>

namespace Moon {

}

namespace MoonInternal {
    void setupModelEngine( std::string state );
}

#else

#include "types.h"

void bind_graph_node(int modelId, struct GraphNode *graphNode);
struct GraphNode * get_graph_node(int modelId);

#endif
#endif