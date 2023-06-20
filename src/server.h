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

class Args
{
private:
    /* data */
public:
    const std::vector<std::string> vars;
    const std::string query;
    httpMethods method;
    Session session;
    Args(std::vector<std::string> vars_f, httpMethods method_f, Session session_f)
        : vars(vars_f), method(method_f), session(session_f) {}
};

class HttpServer
{
private:
    int handleRequest(int socket);
    int sendResponse(int socket, std::string response);
    int sendResponse(int socket, std::vector<std::string> response);
    // Definición de la estructura Route
    struct Route
    {
        std::string path;
        std::vector<std::string> methods;
        std::function<std::string(Args&)> handler;
    };

    struct RouteFiles
    {
        std::string path;
        std::string folder_path;
    };

    std::vector<Route> routes;
    std::vector<RouteFiles> routes_file;
    
    void createSession();

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

    void addFilesHandler(const std::string &path, const std::string &folder_path);
};

#endif // SERVER_H