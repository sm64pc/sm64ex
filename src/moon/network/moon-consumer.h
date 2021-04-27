#ifndef MoonCURL
#define MoonCURL

#include <curl/curl.h>
#include <string>
#include <vector>

struct MoonRequest {    
    std::string url;
    std::string body;
    std::vector<std::string> headers;
    std::string file;
};

struct MoonResponse {
    int code;
    std::string error;
    std::string body;
};

enum MoonResponseType {
    Text, File
};

class MoonConsumer {
private:
    CURL *curl;
public:
    void Init();
    void Post(MoonRequest request, MoonResponse* response);
    void Get(MoonRequest request, MoonResponse* response);
};

#endif
