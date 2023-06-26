#include "server.h"
#include "httpProto.h"

std::string Redirect(int socket, std::string url, std::vector<std::string> cookie){///////////areglas cookie
    httpProtoResponse response;

    if(cookie.empty())
        HttpServer::sendResponse(socket, response.defaultRedirect(url));
    else
        HttpServer::sendResponse(socket, response.defaultRedirect_cookie(url, cookie.data()));
    
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