#ifndef RM_RAPI_H
#define RM_RAPI_H

#include "engine/graph_node.h"

class rm_wapi;

class rm_mesh
{
public:
    virtual void preload() = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void cleanup() = 0;
    virtual void render() = 0;
};

struct RMMeshGraphNode
{
    struct GraphNode node;
    rm_mesh* mMesh;
};

class rm_rapi
{
public:
    virtual void setWAPI(rm_wapi* wapi) = 0;
    virtual bool checkSupport() = 0;
    virtual bool init() = 0;
    virtual rm_mesh* createMesh() = 0;

};

#endif