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

#include "httpMethods.h"
#include "session.h"
#include "httpProto.h"
#include "idGenerator.h"
#include "args.h"


#define BUFFER_SIZE 1024

//const std::string SERVER_VERSION = "Soria/0.0.1b (Unix)";

const std::string REDIRECT = "VOID*REDIRECT";

const std::map<std::string, std::string> content_type = {
    {"js", "application/javascript"},
    {"css", "text/css"},
    {"html", "text/html"},
    {"txt", "text/plain"}};

class HttpServer
{
private:
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

    bool HTTPS = false;
    SSLcontext context;
    SSL_CTX *ssl_ctx;

    struct sockaddr_in serverAddress, clientAddress;
    int addrlen = sizeof(serverAddress);

    std::vector<Route> routes;
    std::vector<Session> sessions;
    std::string __not_found = "<h1>NOT FOUND</h1>";

    std::string __render_line(std::string line, std::map<std::string, std::string> data = std::map<std::string, std::string>());
    void __route_to(std::string endpoint, std::string extension);

    int __find_match_session(std::string id);
    Session __get_session(int index);

    int __handle_request(int socket, SSL *ssl);
    int __create_response(SSL *ssl, const std::string &response, Session &session);
    void __startListenerSSL();

public:
    int port = 8080;
    int serverSocket;

    int MAX_CONNECTIONS = 10;
    std::string default_session_name = "SessionID";

    HttpServer(int port_server, int max_connections = 10) : port(port_server), MAX_CONNECTIONS(max_connections) {}
    HttpServer(int port_server, const std::string SSLcontext_server[], int max_connections = 10)
        : port(port_server), MAX_CONNECTIONS(max_connections)
    {
        context.certificate = SSLcontext_server[0];
        context.private_key = SSLcontext_server[1];
        HTTPS = true;
    }

    void startListener();
    int setup();
    void setNotFound(std::string content){__not_found = content;};
    Session setNewSession(Session session);

    void addRoute(const std::string &path,
                  function<std::string(Args &)> handler,
                  std::vector<std::string> methods);

    static int __send_response(SSL *ssl, const std::string &response);

    void urlfor(const std::string &endpoint);
    std::string Render(const std::string &route, std::map<std::string, std::string> data = std::map<std::string, std::string>());

};

std::string Redirect(int socket, std::string url);
std::string Redirect(SSL *ssl, std::string url);
std::string findCookie(HttpServer &server);

#endif // SERVER_H