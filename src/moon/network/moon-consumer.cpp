#ifndef DISABLE_CURL_SUPPORT
#include "moon-consumer.h"

using namespace std;

void MoonConsumer::Init(){
    curl_global_init(CURL_GLOBAL_DEFAULT);
    this->curl = curl_easy_init();

    if(!this->curl) {
        this->curl = NULL;
        curl_global_cleanup();
    }
}

size_t MoonWriteString(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

size_t MoonWriteFile(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void MoonConsumer::Post(MoonRequest request, MoonResponse* response){
    if(!this->curl) {
        response->code = 510;
        response->error = "Failed to get curl instance";
        return;
    }

    FILE *tmp;
    bool isFile = !request.file.empty();

    curl_easy_setopt(this->curl, CURLOPT_URL,        request.url.c_str());
    curl_easy_setopt(this->curl, CURLOPT_NOBODY,     false);
    curl_easy_setopt(this->curl, CURLOPT_VERBOSE,    false);
    curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, request.body.c_str());

    if(isFile){
        tmp = fopen(request.file.c_str(), "wb");
        curl_easy_setopt(this->curl, CURLOPT_WRITEDATA,     tmp);
        curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, MoonWriteFile);
    } else {
        curl_easy_setopt(this->curl, CURLOPT_WRITEDATA,     &response->body);
        curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, MoonWriteString);
    }

    struct curl_slist *chunk = NULL;
    for (std::string::size_type i = 0; i < request.headers.size(); i++) {
        curl_slist_append(chunk, request.headers[i].c_str());
    }

    curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, chunk);
    CURLcode res;

    try {
        res = curl_easy_perform(this->curl);
    } catch (const std::exception& e) {
        response->code = 510;
        response->error = string(e.what());
    }
    if(res == CURLE_OK) {
        curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &response->code);
    } else {
        response->code = 510;
        response->error = curl_easy_strerror(res);
    }

    if(isFile) fclose(tmp);
    curl_easy_cleanup(this->curl);
}

void MoonConsumer::Get(MoonRequest request, MoonResponse* response){
    if(!this->curl) {
        response->code = 510;
        response->error = "Failed to get curl instance";
        return;
    }

    FILE *tmp;
    bool isFile = !request.file.empty();

    curl_easy_setopt(this->curl, CURLOPT_URL,           request.url.c_str());
    curl_easy_setopt(this->curl, CURLOPT_NOBODY,        false);
    curl_easy_setopt(this->curl, CURLOPT_VERBOSE,       false);

    if(isFile){
        tmp = fopen(request.file.c_str(), "wb");
        curl_easy_setopt(this->curl, CURLOPT_WRITEDATA,     tmp);
        curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, MoonWriteFile);
    } else {
        curl_easy_setopt(this->curl, CURLOPT_WRITEDATA,     &response->body);
        curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, MoonWriteString);
    }

    struct curl_slist *chunk = NULL;
    for (std::string::size_type i = 0; i < request.headers.size(); i++) {
        curl_slist_append(chunk, request.headers[i].c_str());
    }

    curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, chunk);
    CURLcode res;

    try {
        res = curl_easy_perform(this->curl);
    } catch (const std::exception& e) {
        response->code = 510;
        response->error = string(e.what());
    }
    if(res == CURLE_OK) {
        curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &response->code);
    } else {
        response->code = 510;
        response->error = curl_easy_strerror(res);
    }
    if(isFile) fclose(tmp);
    curl_easy_cleanup(this->curl);
}
#endif