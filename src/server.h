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

class Args
{
private:
    /* data */
public:
    const std::vector<std::string> vars;
    const std::string query;
    httpMethods method;
    Session session;
    int socket;
    Args(std::vector<std::string> vars_f, httpMethods method_f, Session session_f)
        : vars(vars_f), method(method_f), session(session_f) {}
};

class HttpServer
{
private:
    int handleRequest(int socket);
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

    int MAX_CONNECTIONS = 10;
    std::vector<Session> sessions;
    Session findMatchSession(std::string id);

    HttpServer(int port_server) : port(port_server) {}

    void startListener(int port);
    int setup();

    static int sendResponse(int socket, std::string response);
    int sendResponse(int socket, std::vector<std::string> response);

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

std::string Redirect(int socket, std::string url);

#endif // SERVER_H