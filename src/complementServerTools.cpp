#include "server.h"
#include "httpProto.h"

std::string Redirect(int socket, std::string url){
    httpProtoResponse response;
    
    HttpServer::sendResponse(socket, response.defaultRedirect(url));

    return REDIRECT;
}

bool starts_with_prefix(const std::string& url){
    std::string prefix("://");
    if (url.size() <= prefix.size())
        return false;
    int index = url.find(prefix);
    if(index != -1)
        return url.substr(index , prefix.size()) == prefix;
    else
        return false;
}