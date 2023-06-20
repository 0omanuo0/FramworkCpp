#ifndef HTTP_METHOD_H
#define HTTP_METHOD_H

#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <regex>

const std::string GET = "get";
const std::string POST = "post";

class httpMethods
{
private:
    void loadParamsGET(std::string request);
    void loadParamsPOST(std::string request);
public:
    std::string type;
    std::string route;
    std::string query;

    struct httpParamsGET
    {
        bool is_GET;
        std::string host;
        std::string user_agent;
        std::vector<std::string> accept;
        std::vector<std::string> accept_language;
        std::vector<std::string> accept_encoding;
        std::map<std::string, std::string> cookies;
        std::string DNT;
        std::string connection;
        std::string upgrade_insecure_requests;
    };

    struct httpParamsPOST
    {
        bool is_POST;

        std::string host;
        std::string user_agent;
        std::vector<std::string> accept;
        std::vector<std::string> accept_language;
        std::vector<std::string> accept_encoding;
        std::string content_type;
        std::string origin;
        std::string referer;

        std::map<std::string, std::string> content;
    };

    httpParamsGET params_get;
    httpParamsPOST params_post;

    httpMethods()
    {
        params_get.is_GET = false;
        params_post.is_POST = false;
    }

    httpMethods(std::string type_h)
        : type(type_h)
    {
        params_get.is_GET = false;
        params_post.is_POST = false;
        if (type_h == GET)
            params_get.is_GET = true;
        if (type_h == POST)
            params_post.is_POST = true;
    };
    

    int loadParams(std::string request);
};
#endif
