#include "httpMethods.h"
#include <iostream>


int httpMethods::loadParams(std::string request)
{

    size_t start = request.find(" ") + 1;
    size_t end = request.find(" ", start);
    route = request.substr(start, end - start);

    std::size_t pos = route.find('?');

    if (pos != std::string::npos){
        route = route.substr(0, pos);
        if (pos+1 >= route.length())
            query = route.substr(pos+1);
    }

    if (type.empty())
    {
        type = request.substr(0, request.find(" "));
        if (type == "GET")
        {
            type = GET;
            params_get.is_GET = true;
        }
        else if (type == "POST")
        {
            type = POST;
            params_post.is_POST = true;
        }
    }
    if(type == GET)
        loadParamsGET(request);
    else if (type == POST)
        loadParamsPOST(request);
    return 0;
}

void httpMethods::loadParamsGET(std::string request)
{
    // Extraer la ruta de la solicitud HTTP
    size_t start = request.find(" ") + 1;
    size_t end = request.find(" ", start);

    start = request.find("\r\n") + 2;
    end = request.find("\r\n", start);
    
    while (end != std::string::npos) {
        size_t paramStart = start;
        size_t paramEnd = request.find(" ", paramStart);
        std::string param = request.substr(paramStart, paramEnd - paramStart);
    
        size_t valueStart = paramEnd + 1;
        size_t valueEnd = end;
        std::string value = request.substr(valueStart, valueEnd - valueStart);

        if (param == "Host:")
            params_get.host = value;
        else if (param == "User-Agent:")
            params_get.user_agent = value;
        else if (param == "Accept:")
        {
            std::istringstream iss(value);
            std::string token;
            while (std::getline(iss, token, ','))
                params_get.accept.push_back(token);
        }
        else if (param == "Cookie:")
        {
            std::istringstream iss(value);
            std::string token;
            while (std::getline(iss, token, ';')) {
                std::string key = token.substr(0, token.find("="));
                std::string value2;
                size_t equalsPos = token.find("=");
                if (equalsPos != std::string::npos) {
                    value2 = token.substr(equalsPos + 1);
                }
                params_get.cookies.insert(std::make_pair(key, value2));
            }
            for (const auto& pair : params_get.cookies) {
                std::cout << pair.first << ": " << pair.second << std::endl;
            }
        }
        else if (param == "Accept-Language:")
        {
            std::istringstream iss(value);
            std::string token;
            while (std::getline(iss, token, ','))
                params_get.accept_language.push_back(token);
        }
        else if (param == "Accept-Encoding:")
        {
            std::istringstream iss(value);
            std::string token;
            while (std::getline(iss, token, ','))
                params_get.accept_encoding.push_back(token);
        }
        else if (param == "DNT:")
            params_get.DNT = value;
        else if (param == "Connection:")
            params_get.connection = value;
        else if (param == "Upgrade-Insecure-Requests:")
            params_get.upgrade_insecure_requests = value;

        start = end + 2;
        end = request.find("\r\n", start);
    }
}

void httpMethods::loadParamsPOST(std::string request)
{
    // Extraer la ruta de la solicitud HTTP
    std::string content_lengh;

    size_t start = request.find(" ") + 1;
    size_t end = request.find(" ", start);
    route = request.substr(start, end - start);

    start = request.find("\r\n") + 2;
    end = request.find("\r\n", start);

    while (end != std::string::npos) {
        std::string param = request.substr(start, end - start);
        param = param.substr(0, param.find(" "));
        std::string value = request.substr(request.find(param) + param.length() + 1, end - (request.find(param) + param.length())-1);

        if (param == "Host:")
            params_post.host = value;
        else if (param == "User-Agent:")
            params_post.user_agent = value;
        else if (param == "Accept:")
        {
            std::istringstream iss(value);
            std::string token;
            while (std::getline(iss, token, ','))
                params_post.accept.push_back(token);
        }
        else if (param == "Accept-Language:")
        {
            std::istringstream iss(value);
            std::string token;
            while (std::getline(iss, token, ','))
                params_post.accept_language.push_back(token);
        }
        else if (param == "Accept-Encoding:")
        {
            std::istringstream iss(value);
            std::string token;
            while (std::getline(iss, token, ','))
                params_post.accept_encoding.push_back(token);
        }
        else if (param == "Content-Type:")
            params_post.content_type = value;
        else if (param == "Origin:")
            params_post.origin = value;
        else if (param == "Referer:")
            params_post.referer = value;
        else if (param == "Content-Length:")
            content_lengh = value;

        start = end + 2;
        end = request.find("\r\n", start);
    }

    std::regex pattern("([^=&]+)=([^&]*)");
    std::smatch matches;

    std::string input = request.substr(request.length() - std::stoi(content_lengh), std::stoi(content_lengh));

    while (std::regex_search(input, matches, pattern)) {
        params_post.content[matches[1].str()] = matches[2].str();
        input = matches.suffix();
    }
}
