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
#include "httpMethods.h"
#include "session.h"

const std::string SERVER = "Soria/0.0.1b (Unix)";

const std::string REDIRECT = "VOID*REDIRECT";

const std::map<std::string, std::string> content_type = {
    {"js", "application/javascript"},
    {"css", "text/css"},
    {"html", "text/html"},
    {"txt", "text/plain"}
    };

struct SSLcontext{
    std::string certificate;
    std::string private_key;
};

class Args
{
private:
    /* data */
public:
    const std::vector<std::string> vars;
    const std::string query;
    httpMethods request;
    Session session;
    SSL *ssl;
    int socket;
    Args(std::vector<std::string> vars_f, httpMethods method_f, Session session_f, SSL *ssl_f = NULL)
        : vars(vars_f), request(method_f), session(session_f), ssl(ssl_f) {}
};

class HttpServer
{
private:
    void __startListener(int port);
    void __startListenerSSL(int port);
    int handleRequest(int socket, SSL *ssl);
    // Definición de la estructura Route
    struct Route
    {
        std::string path;
        std::vector<std::string> methods;
        std::function<std::string(Args &)> handler;
    };

    struct RouteFolder
    {
        std::string path;
        std::string folder_path;
    };
    struct RouteFile
    {
        std::string path;
        std::string type;
    };

    SSLcontext context;
    bool HTTPS = false;

    std::vector<Route> routes;
    std::vector<RouteFile> routes_files;
    std::vector<RouteFolder> routes_folder;

    void createSession();
    std::string __render_line(std::string line, std::map<std::string, std::string> data = std::map<std::string, std::string>());

public:
    int port = 8080;
    int serverSocket;
    struct sockaddr_in serverAddress, clientAddress;
    char buffer[1024] = {0};
    int addrlen = sizeof(serverAddress);

    SSL_CTX *ssl_ctx;

    int MAX_CONNECTIONS = 10;
    std::vector<Session> sessions;
    Session findMatchSession(std::string id);

    static int sendResponse(SSL *ssl, std::string responsel);
    static int sendResponse(SSL *ssl, std::vector<std::string> response);

    static int sendResponse(int socket, std::string response);
    static int sendResponse(int socket, std::vector<std::string> response);


    HttpServer(int port_server) : port(port_server) {}
    HttpServer(int port_server, SSLcontext SSLcontext_server)
        : port(port_server), context(SSLcontext_server) {
        HTTPS = true;
    }
    HttpServer(int port_server, const std::string SSLcontext_server[]) 
        : port(port_server) {
        context.certificate = SSLcontext_server[0];
        context.private_key = SSLcontext_server[1];
        HTTPS = true;
    }

    void startListener(int port);
    int setup();

    void addRoute(const std::string &path,
                  std::function<std::string(Args &)> handler,
                  std::vector<std::string> methods);
    void addRoute(const std::string &path,
                  std::function<std::string(Args &)> handler,
                  std::vector<std::string> methods,
                  std::vector<std::string> vars);
    void addRoute(const std::string &path,
                  std::function<std::string(Args &)> handler,
                  std::vector<std::string> methods,
                  std::vector<std::string> vars,
                  std::string query);

    void addrouteFile(const std::string &path, std::string type);
    void addFilesHandler(const std::string &path, const std::string &folder_path);

    void urlfor(const std::string &endpoint);
    std::string render(const std::string &route, std::map<std::string, std::string> data = std::map<std::string, std::string>());
};

std::string Redirect(int socket, std::string url, std::vector<std::string> cookie = {});
std::string Redirect(SSL *ssl, std::string url, std::vector<std::string> cookie = {});
std::string findCookie(HttpServer &server);

#endif // SERVER_H