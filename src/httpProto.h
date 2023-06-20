#pragma once

#include <string>
#include <map>
#include <vector>
#include "server.h"

const std::string COOKIE = "get";
const std::string USERAGENT = "post";

const std::string OK200 = "HTTP/1.1 200 OK";
const std::string NOTFOUND404 = "HTTP/1.1 404 Not Found";
const std::string REDIRECT302 = "HTTP/1.1 302 Found";
const std::string REDIRECT303 = "HTTP/1.1 303 See Other";

class httpProtoResponse
{
private:
public:
    std::map<std::string, std::string> headers;
    int length = 0;
    std::string server = SERVER;

    httpProtoResponse(/* args */){};
    httpProtoResponse(std::map<std::string, std::string> headers_f, std::string server_f = SERVER)
        : server(server_f), headers(headers_f){};

    void appendParam(std::string name, std::string value);
    void appendCookie(std::string cookie[2], std::string path = "/", int max_age = 31536000, bool http_only = true)
    {
        std::string cookie_s = cookie[0] + "=" + cookie[1] + "; Path=" + path + "; max-Age=" + std::to_string(max_age) + (http_only ? "; HttpOnly" : "");
        headers.insert(make_pair("Set-Cookie:", cookie_s));
    }

    std::string createResponseString(std::string type);
    std::string createResponseString(std::string type, std::pair<std::string, std::string> content);

    std::string defaultRedirect(std::string url);
    std::string defaultOK();
    std::string defaultNotFound();
    std::string defaultOK_cookie(std::string cookie[2], std::string path = "/", int max_age = 31536000, bool http_only = true)
    {
        appendCookie(cookie, path, max_age, http_only);
        return defaultOK();
    }
};

#pragma region funciones_utiles
void httpProtoResponse::appendParam(std::string name, std::string value)
{
    headers.insert(make_pair(name, value));
}
/*
void httpProtoResponse::appendCookie(std::string cookie[2], std::string path = "/", int max_age = 31536000, bool http_only = true)
{
    std::string cookie_s = cookie[0] + "=" + cookie[1] + "; Path=" + path + "; max-Age=" + std::to_string(max_age) + (http_only ? "; HttpOnly" : "");
    headers.insert(make_pair("Set-Cookie:", cookie_s));
}

std::string httpProtoResponse::defaultOK_cookie(std::string cookie[2], std::string path = "/", int max_age = 31536000, bool http_only = true)
{
    appendCookie(cookie, path, max_age, http_only);
    return defaultOK();
}
*/
std::string httpProtoResponse::defaultRedirect(std::string url)
{
    headers.insert(std::make_pair("Location", url));
    return createResponseString(REDIRECT303);
}

std::string httpProtoResponse::defaultOK()
{
    headers.insert(std::make_pair("Content-Type", "text/html"));
    return createResponseString(OK200);
}
std::string httpProtoResponse::defaultNotFound()
{
    headers.insert(std::make_pair("Content-Type", "text/html"));
    return createResponseString(NOTFOUND404);
}

std::string httpProtoResponse::createResponseString(std::string type, std::pair<std::string, std::string> content)
{
    headers.insert(content);
    return createResponseString(type);
}

#pragma endregion

// funcion principal que crea la respouesta
std::string httpProtoResponse::createResponseString(std::string type)
{
    std::string response = type + "\r\n";
    if (length > 0)
        headers.insert(std::make_pair("Content-Length", std::to_string(length)));

    if (headers["Content-Type"].empty())
        headers.insert(std::make_pair("Content-Type", "text/html"));

    headers.insert(std::make_pair("Server", server));

    for (const auto &header : headers)
        response += header.first + ": " + header.second + "\r\n";

    return response + "\r\n\n";
}
