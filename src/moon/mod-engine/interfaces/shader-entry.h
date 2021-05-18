#ifndef ShaderEntry
#define ShaderEntry

enum ShaderType {
    VERTEX, FRAGMENT
};

struct Shader {
    std::string fragmentData;
    std::string vertexData;
};

#endif