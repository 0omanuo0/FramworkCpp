#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <functional>
#include <vector>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fstream>
#include <filesystem>

#include "httpMethods.h"
#include "session.h"
#include "httpProto.h"
#include "idGenerator.h"
#include "args.h"
#include "templating.h"

#ifndef SERVER_VALUES
#define SERVER_VALUES

#define BUFFER_SIZE 1024
const std::string SERVER_VERSION = "Soria/0.0.2b (Unix)";
const std::string REDIRECT = "VOID*REDIRECT";

#endif

#ifndef FILE_TYPE
#define FILE_TYPE
const std::map<std::string, std::string> content_type = {
    {"js", "application/javascript"},
    {"css", "text/css"},
    {"html", "text/html"},
    {"txt", "text/plain"}};
#endif

#pragma region structs
struct SSLcontext
{
    std::string certificate;
    std::string private_key;
};
struct Route
{
    std::string path;
    std::vector<std::string> methods;
    std::function<std::string(Args &)> handler;
};
struct RouteFile
{
    std::string path;
    std::string type;
};
#pragma endregion


class HttpServer
{
private:
    bool HTTPS = false;
    SSLcontext context;
    SSL_CTX *ssl_ctx;

    struct sockaddr_in serverAddress, clientAddress;
    int addrlen = sizeof(serverAddress);

    std::vector<Route> routes;
    std::vector<RouteFile> routesFile;
    std::vector<Session> sessions;
    std::string __not_found = "<h1>NOT FOUND</h1>";

    Templating* template_render;

    int __find_match_session(std::string id);
    Session __get_session(int index);

    int __handle_request(int socket, SSL *ssl);

    int __response_create(SSL *ssl, const std::string &response, Session &session);
    int __response_file  (SSL *ssl, const std::string &path, const std::string &type);
    int __response_folder(SSL *ssl, const std::string &response, Session &session);

    int __send_response(SSL *ssl, const std::string &response);
    int __send_response(SSL *ssl, const std::vector<std::string>&response);
    void __startListenerSSL();

public:
    int port = 8080;
    int serverSocket;

    int MAX_CONNECTIONS = 10;
    std::string default_session_name = "SessionID";
    HttpServer();
    HttpServer(int port_server, int max_connections = 10);
    HttpServer(int port_server, const std::string SSLcontext_server[], int max_connections = 10);

    void startListener();
    int setup();
    void setNotFound(std::string content) { __not_found = content; };
    Session setNewSession(Session session);

    void addRoute(const std::string &path,
                  function<std::string(Args &)> handler,
                  std::vector<std::string> methods);

    void addRouteFile(const std::string &endpoint, const std::string &extension);

    void urlfor(const std::string &endpoint);
    std::string Render(const std::string &route, std::map<std::string, std::string> data = std::map<std::string, std::string>()){
        return template_render->Render(route, data);
    };
};



std::string Redirect(int socket, std::string url);
std::string Redirect(SSL *ssl, std::string url);
std::string findCookie(HttpServer &server);

#endif // SERVER_H